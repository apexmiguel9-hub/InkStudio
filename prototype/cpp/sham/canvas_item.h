// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * canvas_item.h — Canvas/CanvasItem stubs for the alpha.
 *
 * In Inkscape the canvas items (Drawing, Catchall, knots…) are the actual
 * event grabbers: a press on a knot is delivered to the knot, not the tool.
 * The alpha has one event consumer (the tool), so the CanvasItem grab/ungrab
 * protocol is kept for port fidelity but is a no-op: the Renderer's touch
 * dispatch already routes handle-hits (SelTrans) before item/tool hits.
 */
#ifndef SHAM_CANVAS_ITEM_H
#define SHAM_CANVAS_ITEM_H

#include "canvas_events.h" // Events::EventMask
#include <string>

struct CanvasItem {
    virtual ~CanvasItem() = default;
    void grab(Inkscape::UI::Events::EventMask /*mask*/) {}
    void ungrab() {}
};

// ui/widget/canvas.h stub — only the surface tools read (autoscroll etc).
class Canvas {
public:
    void enable_autoscroll() {}
    void grab_focus() {}
};

#endif // SHAM_CANVAS_ITEM_H