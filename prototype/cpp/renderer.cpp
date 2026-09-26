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

// Xform (x'=a·x+c·y+e / y'=b·x+d·y+f) -> tvg::Matrix
// (x'=e11·x+e21·y+e31 / y'=e12·x+e22·y+e32), confirmed against
// thorvg-v112.h Matrix {e11..e33}.
static tvg::Matrix toTvg(Geom::Xform const &xf)
{
    tvg::Matrix m;
    m.e11 = (float)xf.a; m.e12 = (float)xf.b; m.e13 = 0.0f;
    m.e21 = (float)xf.c; m.e22 = (float)xf.d; m.e23 = 0.0f;
    m.e31 = (float)xf.e; m.e32 = (float)xf.f; m.e33 = 1.0f;
    return m;
}

static bool isIdentity(Geom::Xform const &xf)
{
    return xf.a == 1.0 && xf.b == 0.0 && xf.c == 0.0 && xf.d == 1.0 &&
           xf.e == 0.0 && xf.f == 0.0;
}

void Renderer::frame()
{
    ensureCanvas();
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
        // per-item bbox cue (Inkscape's "selection bbox", #2A2D35)
        for (SPItem *item : desktop->getSelection()->items()) {
            auto *r = dynamic_cast<SPRect *>(item);
            if (!r || r->isEmpty()) continue;
            Geom::Rect b = r->docBBox();
            auto *shape = tvg::Shape::gen();
            shape->appendRect((float)b.min().x, (float)b.min().y,
                              (float)b.width(), (float)b.height(), 0.0f, 0.0f);
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
    tvg::Result r2 = canvas->draw(false);
    tvg::Result r3 = canvas->sync();
    if (r1 != tvg::Result::Success || r2 != tvg::Result::Success ||
        r3 != tvg::Result::Success) {
        LOGI("frame: update=%d draw=%d sync=%d", (int)r1, (int)r2, (int)r3);
    }
}