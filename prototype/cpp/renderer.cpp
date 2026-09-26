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

#include <EGL/egl.h>
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

    // Attach the GL target: EGL context/surface currently bound by
    // GLSurfaceView, id 0 = the main (default) surface/FBO. Done every frame
    // so a recreated EGL context (surface re-created on resume/rotation)
    // always re-targets the fresh context.
    EGLDisplay dpy = eglGetCurrentDisplay();
    EGLSurface srf = eglGetCurrentSurface(EGL_DRAW);
    EGLContext ctx = eglGetCurrentContext();
    if (dpy && srf && ctx) {
        canvas->target(dpy, srf, ctx, 0, (uint32_t)W, (uint32_t)H,
                       tvg::ColorSpace::ABGR8888);
    }

    // Rebuild the scene every frame from the live registry (alpha
    // "document"): confirmed rects + the tool-tracked preview rect.
    // remove(nullptr) frees the paints the canvas owns from the previous
    // frame; add() transfers ownership of the newly built shapes.
    //
    // NOTE: no direct GL calls in this file on purpose — a grey background
    // rect covers the surface instead of glClearColor/glClear (the NDK
    // link treats gl* as linker-local 8-byte objects and would jump into
    // unmapped data; eglGetCurrent* DO resolve properly via PLT).
    canvas->remove(nullptr);

    auto *bg = tvg::Shape::gen();
    bg->appendRect(0.0f, 0.0f, (float)W, (float)H, 0.0f, 0.0f);
    bg->fill(0xEF, 0xEF, 0xF4, 255); // 0.949f grey, same as the old clear
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

    canvas->draw(true);
    canvas->sync();
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