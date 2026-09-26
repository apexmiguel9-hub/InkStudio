// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * renderer.cpp — ThorVG GL canvas + touch -> CanvasEvent synthesis.
 *
 * Written against ThorVG v1.1.2 (the API pinned in scripts/build_prototype.sh):
 *   Initializer::init(threads)  — global init, no engine argument
 *   GlCanvas::gen()             — raw pointer
 *   Canvas::target(egl…, id=0)  — attach the current EGL surface (id 0 = main)
 *   Canvas::add()/remove()      — paints are owned by the canvas (remove all)
 *   Canvas::draw(clear)/sync()
 *
 * Touch coords from Java (top-left origin, y-down, 1:1 with the doc in the
 * alpha) are synthesized into the tool's CanvasEvent interface (ButtonPress =
 * touch down, Motion with BUTTON1_MASK = drag, ButtonRelease = finger up),
 * the same normalized interface the Inkscape tool exposes.
 *
 * Dispatch (down/move/up) mirrors Inkscape's canvas grab precedence:
 *   1. a visible SelTrans handle under the finger (select tool) — the handle
 *      drag is driven straight by the engine, it never enters the tool;
 *   2. an item under the finger -> tool->item_handler() (if it consumes);
 *   3. otherwise               -> tool->root_handler().
 *
 * The alpha "document" is the live SPRect registry: every frame the renderer
 * redraws all registered rects applying SPRect::xform (identity or the
 * select tool's compose), plus the selection overlay (per-item bbox cue in
 * #2A2D35 + scale/rotate handles) and the rubberband.
 */
#include "renderer.h"

#include <EGL/egl.h>
#include <android/log.h>
#include <cmath>

#include <thorvg.h>

#include "canvas_events.h"
#include "ink_compat.h" // GDK_BUTTON1_MASK
#include "sp_rect.h"    // sprect_registry(), SPRect
#include "desktop.h"    // SPDesktop (shared), SPDocument
#include "selection.h"  // Selection::items() for the cue overlay
#include "rubberband.h" // Rubberband::get() overlay
#include "seltrans.h"   // active_seltrans(), handle overlay engine
#include "rect_tool_port.h"
#include "select_tool_port.h"
#include "node_tool_port.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "inkalpha", __VA_ARGS__)

Renderer::Renderer() = default;

Renderer::~Renderer()
{
    delete tool;
    delete desktop; // owns document singleton? no: document is process-wide
    // canvas is freed by the ThorVG GL context teardown; process-lifetime
    // singleton in practice.
}

void Renderer::recreateTool()
{
    // The tools are cheap; switching tools destroys the old instance (which
    // clears active_seltrans()) and constructs the new one on the SAME
    // desktop — the selection/document state persists across switches.
    delete tool;
    tool = nullptr;
    if (!desktop) {
        desktop = new SPDesktop();
    }
    if (toolId == 1) { // TOOL_RECT
        tool = new Inkscape::UI::Tools::RectTool(desktop);
    } else if (toolId == 2) { // TOOL_NODE
        tool = new Inkscape::UI::Tools::NodeTool(desktop);
    } else {           // 0 = TOOL_SELECT (default)
        tool = new Inkscape::UI::Tools::SelectTool(desktop);
    }
    touchDown = false;
    handleDrag = false;
}

void Renderer::ensureTool()
{
    if (!tool) {
        recreateTool();
    }
}

void Renderer::setTool(int t)
{
    if (t == toolId && tool) {
        return; // already on this tool
    }
    toolId = t;
    recreateTool();
}

void Renderer::ensureCanvas()
{
    if (glReady) return;
    if (tvg::Initializer::init(0) != tvg::Result::Success) {
        LOGI("ThorVG Initializer::init failed");
        return;
    }
    canvas = tvg::GlCanvas::gen();
    if (!canvas) {
        LOGI("GlCanvas::gen() returned null (is libthorvg built with the gl engine?)");
        return;
    }
    glReady = true;
    LOGI("ThorVG GlCanvas ready");
}

void Renderer::resize(int w, int h)
{
    W = w;
    H = h;
}

// Helper: dispatch one synthesized event to the item under p, falling back to
// the root handler when the item handler doesn't consume it (same precedence
// as Inkscape's canvas event delivery).
void Renderer::dispatchEvent(SPItem *item, Inkscape::UI::CanvasEvent const &ev)
{
    if (item && tool->item_handler(item, ev)) {
        return;
    }
    tool->root_handler(ev);
}

void Renderer::touch(float x, float y, int action)
{
    ensureTool();

    Geom::Point const p(x, y);

    if (action == 0) { // DOWN
        if (touchDown) {
            // Safety: complete an unterminated gesture first.
            Inkscape::UI::ButtonReleaseEvent up;
            up.pos = p;
            up.button = 1;
            up.modifiers = 0;
            SPItem *item = desktop->getItemAtPoint(p, false);
            dispatchEvent(item, up);
        }
        touchDown = true;
        desktop->setPoint(p);

        // 1. SelTrans handle (select tool only; active_seltrans() is null
        //    while the rect tool is active). The handle grab never enters the
        //    tool: like Inkscape's knots, it sits above the drawing.
        if (Inkscape::SelTrans *st = Inkscape::active_seltrans(); st && st->tryGrabHandle(p)) {
            handleDrag = true;
            return;
        }

        // 2./3. item -> root
        Inkscape::UI::ButtonPressEvent down;
        down.pos = p;
        down.orig_pos = p;
        down.button = 1;
        down.num_press = 1;
        down.modifiers = 0;
        SPItem *item = desktop->getItemAtPoint(p, false);
        dispatchEvent(item, down);
    } else if (action == 1 && touchDown) { // MOVE
        desktop->setPoint(p);
        if (handleDrag) {
            if (Inkscape::SelTrans *st = Inkscape::active_seltrans(); st) {
                st->moveHandle(p);
            }
            return;
        }
        Inkscape::UI::MotionEvent move;
        move.pos = p;
        move.orig_pos = p;
        move.modifiers = GDK_BUTTON1_MASK; // drag while the finger is down
        SPItem *item = desktop->getItemAtPoint(p, false);
        dispatchEvent(item, move);
    } else if (action == 2) { // UP / CANCEL
        if (!touchDown && !handleDrag) return;
        touchDown = false;
        if (handleDrag) {
            handleDrag = false;
            if (Inkscape::SelTrans *st = Inkscape::active_seltrans(); st) {
                st->releaseHandle();
            }
            return;
        }
        Inkscape::UI::ButtonReleaseEvent up;
        up.pos = p;
        up.button = 1;
        up.modifiers = 0;
        SPItem *item = desktop->getItemAtPoint(p, false);
        dispatchEvent(item, up);
    }
}

// Doc Xform (x'=a·x+c·y+e / y'=b·x+d·y+f) -> tvg::Matrix.
// ThorVG v1.1.2 multiplies points as ROW vectors (tvgMath.h operator*=
// Point&, Matrix&): x' = e11·x + e12·y + e13, y' = e21·x + e22·y + e23,
// so the translation lives in the THIRD COLUMN (e13/e23), not the third
// row. Mapping: a->e11, c->e12, e->e13 / b->e21, d->e22, f->e23.
// The old layout put translation in e31/e32: ThorVG silently dropped it —
// shapes "moved" in the document model but rendered pinned at the origin,
// which is exactly the detached selection-box/overlay bugs users saw.
static tvg::Matrix toTvg(Geom::Xform const &xf)
{
    tvg::Matrix m;
    m.e11 = (float)xf.a; m.e12 = (float)xf.c; m.e13 = (float)xf.e;
    m.e21 = (float)xf.b; m.e22 = (float)xf.d; m.e23 = (float)xf.f;
    m.e31 = 0.0f;        m.e32 = 0.0f;        m.e33 = 1.0f;
    return m;
}

static bool isIdentity(Geom::Xform const &xf)
{
    return xf.a == 1.0 && xf.b == 0.0 && xf.c == 0.0 && xf.d == 1.0 &&
           xf.e == 0.0 && xf.f == 0.0;
}

// ---- Node Tool overlay (Option 1: shim over rect) -----------------------
// Draws 8 scale handles (10x10 white squares with #101014 stroke) on the
// rect's docCorners + mid-edges, plus 4 rotate handles (r=7 white circles
// at 28px diagonal from corners) + center crosshair (dark).
static void drawNodeOverlay(tvg::Canvas *canvas, const SPRect *r)
{
    if (!r || r->isEmpty()) return;
    auto c = r->docCorners();
    Geom::Point c0 = c[0], c1 = c[1], c2 = c[2], c3 = c[3];

    // Scale handles: corners + mid-edges (8 total)
    Geom::Point handles[8] = {
        c0,
        {(c0.x + c1.x) * 0.5, (c0.y + c1.y) * 0.5},
        c1,
        {(c1.x + c2.x) * 0.5, (c1.y + c2.y) * 0.5},
        c2,
        {(c2.x + c3.x) * 0.5, (c2.y + c3.y) * 0.5},
        c3,
        {(c3.x + c0.x) * 0.5, (c3.y + c0.y) * 0.5}
    };

    for (int i = 0; i < 8; ++i) {
        auto *shape = tvg::Shape::gen();
        shape->appendRect((float)handles[i].x - 5.0f, (float)handles[i].y - 5.0f,
                          10.0f, 10.0f, 0.0f, 0.0f);
        shape->fill(255, 255, 255, 255);
        shape->strokeWidth(1.0f);
        shape->strokeFill(0x10, 0x10, 0x14, 255);
        canvas->add(shape);
    }

    // Rotate handles: 4 circles at 28px diagonal from corners
    const double ROT_OFF = 28.0;
    for (int i = 0; i < 4; ++i) {
        Geom::Point next = c[(i + 1) % 4];
        Geom::Point diag = {next.x - c[i].x, next.y - c[i].y};
        double len = std::sqrt(diag.x * diag.x + diag.y * diag.y);
        if (len > 0) {
            diag.x = diag.x / len * ROT_OFF;
            diag.y = diag.y / len * ROT_OFF;
        }
        Geom::Point rh = {c[i].x - diag.x, c[i].y - diag.y};
        auto *shape = tvg::Shape::gen();
        shape->appendCircle((float)rh.x, (float)rh.y, 7.0f, 7.0f);
        shape->fill(255, 255, 255, 255);
        shape->strokeWidth(1.0f);
        shape->strokeFill(0x10, 0x10, 0x14, 255);
        canvas->add(shape);
    }

    // Center crosshair (dark)
    double cx = (c0.x + c1.x + c2.x + c3.x) * 0.25;
    double cy = (c0.y + c1.y + c2.y + c3.y) * 0.25;
    auto *ch1 = tvg::Shape::gen();
    ch1->appendRect((float)cx - 8.0f, (float)cy - 0.75f, 16.0f, 1.5f, 0.0f, 0.0f);
    auto *ch2 = tvg::Shape::gen();
    ch2->appendRect((float)cx - 0.75f, (float)cy - 8.0f, 1.5f, 16.0f, 0.0f, 0.0f);
    ch1->fill(0x10, 0x10, 0x14, 255);
    ch2->fill(0x10, 0x10, 0x14, 255);
    canvas->add(ch1);
    canvas->add(ch2);
}

void Renderer::frame()
{
    ensureCanvas();
    ensureTool(); // first frame has seen no touch yet — tool must exist
    if (!glReady || !tool || W <= 0 || H <= 0) return;

    // GlCanvas requires target() before rendering: without it update()/draw()
    // return Result::InsufficientCondition (2) and nothing is drawn (black).
    // id=0 = the main (default) surface, i.e. exactly what GLSurfaceView
    // presents. Re-target every frame so a recreated EGL context (resume /
    // rotation / surface re-create) always targets the fresh context.
    EGLDisplay dpy = eglGetCurrentDisplay();
    EGLSurface srf = eglGetCurrentSurface(EGL_DRAW);
    EGLContext ctx = eglGetCurrentContext();
    if (dpy && srf && ctx) {
        // ABGR8888S is REQUIRED: GlRenderer::target() v1.1.2 returns
        // NonSupport for any other ColorSpace (observed black canvas when
        // ABGR8888 was passed).
        tvg::Result rt = canvas->target(dpy, srf, ctx, 0, (uint32_t)W,
                                        (uint32_t)H,
                                        tvg::ColorSpace::ABGR8888S);
#ifndef NDEBUG
        if (rt != tvg::Result::Success)
            LOGI("frame: target=%d", (int)rt);
#endif
    } else {
        // Only reachable if eglGetCurrent* fails (no current EGL context)
        LOGI("frame: no current EGL handles (dpy=%d srf=%d ctx=%d)",
             dpy != nullptr, srf != nullptr, ctx != nullptr);
    }

    canvas->viewport(0, 0, W, H);

    // Rebuild the scene every frame from the live registry (alpha
    // "document"): confirmed rects (with their document transform) + the
    // selection overlay + the rubberband. remove(nullptr) frees the paints
    // the canvas owns from the previous frame; add() transfers ownership of
    // the newly built shapes.
    canvas->remove(nullptr);

    auto *bg = tvg::Shape::gen();
    bg->appendRect(0.0f, 0.0f, (float)W, (float)H, 0.0f, 0.0f);
    bg->fill(0xEF, 0xEF, 0xF4, 255); // 0.949f grey, ancient clear colour
    canvas->add(bg);

    for (SPRect *r : sprect_registry()) {
        float x = (float)r->x.computed;
        float y = (float)r->y.computed;
        float w = (float)r->width.computed;
        float h = (float)r->height.computed;
        if (w == 0.0f || h == 0.0f) continue;

        auto *shape = tvg::Shape::gen();
        shape->appendRect(x, y, w, h,
                          (float)r->rx.computed, (float)r->ry.computed);
        shape->fill(0x33, 0x66, 0xFF, 240);        // fill
        shape->strokeWidth(1.5f);
        shape->strokeFill(0x12, 0x14, 0x20, 255);  // stroke

        // Item transform (select tool move/scale/rotate); skip identity.
        if (!isIdentity(r->xform)) {
            shape->transform(toTvg(r->xform));
        }
        canvas->add(shape);
    }

    // ---- selection overlay (only while the select tool is active) -------
    if (Inkscape::SelTrans *st = Inkscape::active_seltrans(); st && !st->isEmpty()) {
        // per-item cue: the *transformed* box outline (rotates with the fill,
        // Inkscape's "_bbox->corner(i) * affine" box) — #2A2D35
        for (SPItem *item : desktop->getSelection()->items()) {
            auto *r = dynamic_cast<SPRect *>(item);
            if (!r || r->isEmpty()) continue;
            auto c = r->docCorners();
            auto *shape = tvg::Shape::gen();
            shape->moveTo((float)c[0].x, (float)c[0].y);
            shape->lineTo((float)c[1].x, (float)c[1].y);
            shape->lineTo((float)c[2].x, (float)c[2].y);
            shape->lineTo((float)c[3].x, (float)c[3].y);
            shape->close();
            shape->strokeWidth(1.0f);
            shape->strokeFill(0x2A, 0x2D, 0x35, 255);
            canvas->add(shape);
        }

        // scale/rotate handles (SCALE: 8 white 10x10 squares; ROTATE: 4 white
        // r=7 circles at the offset corners + center crosshair)
        if (st->visible()) {
            for (auto const &hp : st->handlePositions()) {
                auto *shape = tvg::Shape::gen();
                if (st->state() == Inkscape::SelTrans::STATE_SCALE) {
                    shape->appendRect(hp.pos.x - 5.0f, hp.pos.y - 5.0f,
                                      10.0f, 10.0f, 0.0f, 0.0f);
                } else if (hp.id == Inkscape::SelTrans::Handle::CENTER) {
                    // center crosshair (dark, thin): rotation pivot
                    shape->appendRect(hp.pos.x - 8.0f, hp.pos.y - 0.75f,
                                      16.0f, 1.5f, 0.0f, 0.0f);
                    shape->appendRect(hp.pos.x - 0.75f, hp.pos.y - 8.0f,
                                      1.5f, 16.0f, 0.0f, 0.0f);
                    shape->fill(0x10, 0x10, 0x14, 255);
                    canvas->add(shape);
                    continue;
                } else {
                    shape->appendCircle(hp.pos.x, hp.pos.y, 7.0f, 7.0f);
                }
                shape->fill(255, 255, 255, 255);
                shape->strokeWidth(1.0f);
                shape->strokeFill(0x10, 0x10, 0x14, 255);
                canvas->add(shape);
            }
        }
    }

    // ---- node tool overlay (only while the node tool is active) -------------
    if (toolId == 2) {
        if (auto *nt = dynamic_cast<Inkscape::UI::Tools::NodeTool *>(tool)) {
            if (SPRect *r = nt->getRect()) {
                drawNodeOverlay(canvas, r);
            }
        }
    }

    // ---- rubberband overlay (select tool drag on empty area) ----------
    if (Inkscape::Rubberband *rb = Inkscape::Rubberband::get(desktop);
        rb->isStarted() && rb->isMoved()) {
        if (auto b = rb->getRectangle()) {
            auto *shape = tvg::Shape::gen();
            shape->appendRect((float)b->min().x, (float)b->min().y,
                              (float)b->width(), (float)b->height(), 0.0f, 0.0f);
            shape->fill(0x33, 0x66, 0xFF, 60);       // translucent
            shape->strokeWidth(1.0f);
            shape->strokeFill(0x2E, 0x5F, 0xFF, 255);
            canvas->add(shape);
        }
    }

    tvg::Result r1 = canvas->update();
    // draw(true): clears the target buffer before drawing (Canvas::draw, thorvg.h
    // v1.1.2). Without the clear, the GL blit only overwrites the updated region
    // and the previous frame's pixels survive outside it — dragging/scaling an
    // item left ghost silhouettes of the OLD geometry (the "detached box" look).
    // Our scene is fully opaque (background rect covers the viewport), so the
    // extra clear is invisible when nothing moved and corrects stale pixels
    // otherwise. LVGL-style note: skip is valid only for full-cover scenes with
    // no animation; we rely on it being precise now.
    tvg::Result r2 = canvas->draw(true);
    tvg::Result r3 = canvas->sync();
    if (r1 != tvg::Result::Success || r2 != tvg::Result::Success ||
        r3 != tvg::Result::Success) {
        LOGI("frame: update=%d draw=%d sync=%d", (int)r1, (int)r2, (int)r3);
    }
}