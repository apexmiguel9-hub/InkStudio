// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * renderer.h — ThorVG GL canvas for the alpha prototype.
 *
 * Owns the surface size, the ThorVG canvas, and the active tool. The
 * "document" is the live SPRect registry (sp_rect.h): every frame the
 * renderer redraws all registered rects — the tool-tracked one doubles as
 * the drag preview (it is updated per motion by the ported tool).
 */
#ifndef INKALPHA_RENDERER_H
#define INKALPHA_RENDERER_H

namespace tvg { class GlCanvas; }

namespace Inkscape::UI::Tools { class RectTool; }

class Renderer {
public:
    Renderer();
    ~Renderer();

    void resize(int w, int h);
    void frame();                       // must run on the GL thread
    void touch(float x, float y, int action); // 0=down 1=move 2=up/cancel
    void setTool(int tool);             // 0 = rect (the only tool in the alpha)

private:
    void ensureCanvas();
    void ensureTool();

    int W = 0;
    int H = 0;
    bool glReady = false;
    bool touchDown = false;
    int toolId = 0;
    tvg::GlCanvas *canvas = nullptr;
    Inkscape::UI::Tools::RectTool *tool = nullptr;
};

#endif // INKALPHA_RENDERER_H