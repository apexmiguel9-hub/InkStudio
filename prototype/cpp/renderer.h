// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * renderer.h — ThorVG GL canvas for the alpha prototype.
 *
 * Owns the surface size, the ThorVG canvas, the shared SPDesktop, and the
 * active tool. The "document" is the live SPRect registry (sp_rect.h): every
 * frame the renderer redraws all registered rects applying their Xform, plus
 * the selection overlay (bbox cue + scale/rotate handles) and the rubberband.
 *
 * Touch dispatch (Renderer::touch) mirrors Inkscape's canvas grab order:
 *   1. a visible SelTrans handle under the finger  -> seltrans handle drag
 *   2. an item under the finger                   -> tool->item_handler()
 *   3. otherwise                                  -> tool->root_handler()
 */
#ifndef INKALPHA_RENDERER_H
#define INKALPHA_RENDERER_H

namespace tvg { struct GlCanvas; }

class SPDesktop;
class SPItem;
namespace Inkscape::UI { struct CanvasEvent; }
namespace Inkscape::UI::Tools { class ToolBase; }

class Renderer {
public:
    Renderer();
    ~Renderer();

    void resize(int w, int h);
    void frame();                       // must run on the GL thread
    void touch(float x, float y, int action); // 0=down 1=move 2=up/cancel
    void setTool(int tool);             // 0 = Select (default), 1 = Rect

private:
    void ensureCanvas();
    void ensureTool();
    void recreateTool();

    // item-under-finger first, root fallback (Inkscape grab precedence).
    void dispatchEvent(SPItem *item, Inkscape::UI::CanvasEvent const &ev);

    int W = 0;
    int H = 0;
    bool glReady = false;
    bool touchDown = false;
    bool handleDrag = false; // a SelTrans handle is being dragged
    int toolId = 0;          // TOOL_SELECT by default
    SPDesktop *desktop = nullptr;
    tvg::GlCanvas *canvas = nullptr;
    Inkscape::UI::Tools::ToolBase *tool = nullptr;
};

#endif // INKALPHA_RENDERER_H