// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * node_tool_port.cpp — port of Inkscape ui/tools/node-tool.cpp (alpha, Option 1).
 *
 * Minimal Node Tool for editing SPRect corners.
 * Handles: click node/handle -> grab, drag -> move, release -> ungrab.
 * Reuses SPRect::nodeAt/handleAt/moveNode/moveHandle.
 */
#include "ink_compat.h"     // GDK constants
#include "canvas_events.h"  // CanvasEvent + inspect_event
#include "selection.h"      // Inkscape::Selection
#include "sp_rect.h"        // SPRect + nodeAt/handleAt/moveNode/moveHandle
#include "tool_base.h"      // ToolBase
#include "node_tool_port.h"

namespace Inkscape::UI::Tools {

NodeTool::NodeTool(SPDesktop *desktop)
    : ToolBase(desktop, "/tools/node", "node-tool.svg")
{
    // Pick up the currently selected rect (if any)
    SPItem *item = desktop->getSelection()->singleItem();
    if (item && is<SPRect>(item)) {
        rect = cast<SPRect>(item);
    }
}

NodeTool::~NodeTool() {
    ungrabCanvasEvents();
    cancel();
}

void NodeTool::grabNode(SPRect *r, int idx, Geom::Point pos) {
    if (!r) return;
    rect = r;
    grabbed_node = idx;
    grab_pos = pos;
    grabCanvasEvents();
}

void NodeTool::grabHandle(SPRect *r, int idx, Geom::Point pos) {
    if (!r) return;
    rect = r;
    grabbed_handle = idx;
    grab_pos = pos;
    grabCanvasEvents();
}

void NodeTool::ungrab() {
    grabbed_node = -1;
    grabbed_handle = -1;
    ungrabCanvasEvents();
}

void NodeTool::finishItem() {
    // In the alpha, selection persists; nothing to finalize for node edits
    // (SPRect updates are live via moveNode/moveHandle).
    // Real Inkscape: commits the path changes to the document/undo stack.
    ungrab();
}

void NodeTool::cancel() {
    ungrab();
}

bool NodeTool::item_handler(SPItem *item, CanvasEvent const &event)
{
    // Node tool only operates on selected rect; click on unselected rect
    // should select it first (handled by root_handler via selection).
    return ToolBase::item_handler(item, event);
}

bool NodeTool::root_handler(CanvasEvent const &event)
{
    auto selection = _desktop->getSelection();
    auto prefs = Inkscape::Preferences::get();

    // Drag tolerance for node/handle grab (same as rect tool)
    int tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    bool ret = false;

    inspect_event(event,
        [&] (ButtonPressEvent const &event) {
            if (event.button != 1 || event.num_press != 1) return;

            Geom::Point pos_w = event.pos;
            Geom::Point pos_dt = _desktop->w2d(pos_w);

            // Refresh rect from current selection (user may have switched tools)
            SPItem *sel_item = _desktop->getSelection()->singleItem();
            if (sel_item && is<SPRect>(sel_item)) {
                rect = cast<SPRect>(sel_item);
            }

            // If a rect is selected, test its nodes/handles
            SPRect *r = rect.get();
            if (r && !r->isEmpty()) {
                int node_idx = r->nodeAt(pos_dt.x, pos_dt.y, tolerance);
                if (node_idx >= 0) {
                    grabNode(r, node_idx, pos_dt);
                    ret = true;
                    return;
                }
                int handle_idx = r->handleAt(pos_dt.x, pos_dt.y, tolerance);
                if (handle_idx >= 0) {
                    grabHandle(r, handle_idx, pos_dt);
                    ret = true;
                    return;
                }
            }

            // Click on empty space or body -> could select a rect (delegated to select tool)
            // For alpha: if we click a rect body, SelectTool will handle selection.
            // NodeTool just passes through.
            item_to_select = sp_event_context_find_item(_desktop, pos_w, false, true);
            if (item_to_select) {
                // Let selection happen via normal flow (SelectTool or this tool's base)
                // For now, we don't change selection here.
            }

            saveDragOrigin(pos_w);
            dragging = true;
            center = pos_dt;
            grabCanvasEvents();
            ret = true;
        },
        [&] (MotionEvent const &event) {
            if (!dragging || !(event.modifiers & GDK_BUTTON1_MASK)) return;
            if (!checkDragMoved(event.pos)) return;

            Geom::Point pos_dt = _desktop->w2d(event.pos);
            SPRect *r = rect.get();

            if (r && !r->isEmpty()) {
                if (grabbed_node >= 0) {
                    r->moveNode(grabbed_node, pos_dt);
                    ret = true;
                } else if (grabbed_handle >= 0) {
                    r->moveHandle(grabbed_handle, pos_dt);
                    ret = true;
                }
            }

            if (!ret) {
                // Body drag -> move whole rect (could delegate to SelectTool logic)
                // For alpha: just update center; real impl would transform xform.
                center = pos_dt;
            }

            gobble_motion_events(GDK_BUTTON1_MASK);
            ret = true;
        },
        [&] (ButtonReleaseEvent const &event) {
            if (event.button != 1) return;
            if (grabbed_node >= 0 || grabbed_handle >= 0) {
                finishItem();
            } else if (dragging) {
                cancel();
            }
            ungrab();
            dragging = false;
            ret = true;
        },
        [&] (KeyPressEvent const &event) {
            // Escape = cancel
            if (get_latin_keyval(event) == GDK_KEY_Escape) {
                cancel();
                ungrab();
                ret = true;
            }
        }
    );

    return ret;
}

} // namespace Inkscape::UI::Tools