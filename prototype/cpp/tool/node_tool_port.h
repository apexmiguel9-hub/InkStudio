// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * node_tool_port.h — port of Inkscape ui/tools/node-tool.h (alpha, Option 1).
 *
 * Minimal Node Tool for editing SPRect corners (Option 1: shim over rect).
 * Full path editing (Option 2) will replace this when escalation trigger fires.
 */
#ifndef INKALPHA_TOOLS_NODE_TOOL_H
#define INKALPHA_TOOLS_NODE_TOOL_H

#include "tool_base.h"
#include "sp_rect.h"

namespace Inkscape::UI::Tools {

class NodeTool : public ToolBase
{
public:
    NodeTool(SPDesktop *desktop);
    ~NodeTool() override;

    bool root_handler(CanvasEvent const &event) override;
    bool item_handler(SPItem *item, CanvasEvent const &event) override;

    // For renderer overlay access
    SPRect *getRect() const { return rect.get(); }

private:
    SPWeakPtr<SPRect> rect;          // rect being edited (from selection)
    int grabbed_node = -1;           // 0-3 = corner nodes
    int grabbed_handle = -1;         // 0-7 scale, 8-11 rotate
    Geom::Point grab_pos;            // document-space grab position

    void grabNode(SPRect *r, int idx, Geom::Point pos);
    void grabHandle(SPRect *r, int idx, Geom::Point pos);
    void ungrab();
    void finishItem();
    void cancel();
};

} // namespace Inkscape::UI::Tools

#endif // INKALPHA_TOOLS_NODE_TOOL_H