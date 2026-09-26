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
 * touch down, Motion with BUTTON1_MASK = drag, ButtonRelease = finger up).
 * That is the same normalized mouse interface the Inkscape tool exposes;
 * nothing tool-specific lives here.
 */
#include "renderer.h"

#include <android/log.h>
#include <cmath>

#include <thorvg.h>

#include "ink_compat.h" // GDK_BUTTON1_MASK
#include "sp_rect.h"    // sprect_registry(), SPRect
#include "rect_tool_port.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "inkalpha", __VA_ARGS__)

Renderer::Renderer() = default;

Renderer::~Renderer()
{
    delete tool;
    // canvas is freed by the ThorVG GL context teardown; process-lifetime
    // singleton in practice.
}

void Renderer::ensureTool()
{
    if (!tool) {
        tool = new Inkscape::UI::Tools::RectTool(new SPDesktop());
    }
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

void Renderer::setTool(int t)
{
    toolId = t; // alpha: only TOOL_RECT=0 exists; future tools switch here.
    ensureTool();
}

void Renderer::frame()
{
    ensureCanvas();
    if (!glReady || W <= 0 || H <= 0) return;

    // Official ThorVG GL usage (v1.1.2 test/testGlCanvas.cpp): GlCanvas
    // renders into the currently-bound surface; NO target() call. The
    // pipeline needs update() after add() to process the paints, then
    // draw()/sync(). (My earlier target(egl...) pushed rendering into a
    // hidden FBO and the missing update() left the scene unprocessed —
    // both produced an empty/black canvas.)
    canvas->viewport(0, 0, W, H);

    // Rebuild the scene every frame from the live registry (alpha
    // "document"): confirmed rects + the tool-tracked preview rect.
    // remove(nullptr) frees the paints the canvas owns from the previous
    // frame; add() transfers ownership of the newly built shapes.
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
        canvas->add(shape);
    }

    tvg::Result r1 = canvas->update();
    tvg::Result r2 = canvas->draw(false);
    tvg::Result r3 = canvas->sync();
    if (r1 != tvg::Result::Success || r2 != tvg::Result::Success ||
        r3 != tvg::Result::Success) {
        LOGI("frame: update=%d draw=%d sync=%d", (int)r1, (int)r2, (int)r3);
    }
}

void Renderer::touch(float x, float y, int action)
{
    ensureTool();

    if (action == 0) { // DOWN
        if (touchDown) {
            // Safety: complete an unterminated gesture first.
            Inkscape::UI::ButtonReleaseEvent up;
            up.pos = {x, y};
            up.button = 1;
            up.modifiers = 0;
            tool->root_handler(up);
        }
        touchDown = true;
        Inkscape::UI::ButtonPressEvent down;
        down.pos = {x, y};
        down.button = 1;
        down.num_press = 1;
        down.modifiers = 0;
        tool->root_handler(down);
    } else if (action == 1 && touchDown) { // MOVE
        Inkscape::UI::MotionEvent move;
        move.pos = {x, y};
        move.modifiers = GDK_BUTTON1_MASK; // drag while the finger is down
        tool->root_handler(move);
    } else if (action == 2) { // UP / CANCEL
        if (!touchDown) return;
        touchDown = false;
        Inkscape::UI::ButtonReleaseEvent up;
        up.pos = {x, y};
        up.button = 1;
        up.modifiers = 0;
        tool->root_handler(up);
    }
}