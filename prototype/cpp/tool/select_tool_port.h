// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * select_tool_port.h — port of Inkscape ui/tools/select-tool.h (alpha).
 *
 * Header preserved 1:1 from the original (fields + methods, order kept);
 * only the includes are redirected to the sham/ harness. Two deviations:
 *   - onHideSelectionChanged() keeps its `override`? No: the sham ToolBase
 *     has no such virtual, so the override keyword is dropped (the method is
 *     still wired to the seltrans cue).
 *   - ScrollEvent resolves to the sham's Inkscape::UI::ScrollEvent.
 */
#ifndef INKALPHA_TOOLS_SELECT_TOOL_H
#define INKALPHA_TOOLS_SELECT_TOOL_H

#include "rubberband.h"    // original: rubberband.h
#include "tool_base.h"     // original: ui/tools/tool-base.h
#include "canvas_events.h" // original: ui/widget/events/canvas-event.h
#include "canvas_item.h"   // CanvasItem (grabbed member)
#include "util.h"          // Util::ActionAccel
#include <string>
#include <utility>
#include <vector>

class SPDesktop;

namespace Inkscape {
class SelTrans;
class SelectionDescriber;
class Selection;
} // namespace Inkscape

namespace Inkscape::UI::Tools {

class SelectTool : public ToolBase
{
public:
    SelectTool(SPDesktop *desktop);
    ~SelectTool() override;

    bool moved = false;
    unsigned button_press_state = 0;

    std::vector<SPItem *> cycling_items;
    std::vector<SPItem *> cycling_items_cmp;
    SPItem *cycling_cur_item = nullptr;
    bool cycling_wrap = true;

    SPItem *item = nullptr;
    CanvasItem *grabbed = nullptr;
    Inkscape::SelTrans *_seltrans = nullptr;
    Inkscape::SelectionDescriber *_describer = nullptr;
    char *no_selection_msg = nullptr;

    void set(Inkscape::Preferences::Entry const &val) override;
    bool root_handler(CanvasEvent const &event) override;
    bool item_handler(SPItem *item, CanvasEvent const &event) override;

    void updateDescriber(Inkscape::Selection *sel);

private:

    // aborts selection interaction
    bool sp_select_context_abort();

    void sp_select_context_cycle_through_items(Inkscape::Selection *selection,
                                               ScrollEvent const &scroll_event);

    // resets the opacities of all selected items to their original values
    void sp_select_context_reset_opacities();

    static std::pair<Rubberband::Mode, CanvasItemCtrlType> get_default_rubberband_state();

    void _duplicate_drag(Geom::Point const &p);
    bool _duplicate_drag_state(unsigned int state) const;
    void _duplicate_drag_reset();
    bool _duplicate_drag_on_press = false;
    bool _duplicate_down_on_selected = false;

    void handleClick(ButtonReleaseEvent const &event, Inkscape::Selection *selection);

    bool _alt_on = false;
    bool _force_dragging = false;

    Geom::Point _live_point;

    std::string _default_cursor;
    // original: `void onHideSelectionChanged(bool hide) override;` — the sham
    // ToolBase has no such virtual, so no override (kept for the alpha HUD).
    void onHideSelectionChanged(bool hide);

    Util::ActionAccel _acc_st_grab;
    Util::ActionAccel _acc_st_scale;
    Util::ActionAccel _acc_st_rotate;

    Modifiers::Modifier *mod_select_add_to;
    Modifiers::Modifier *mod_select_always_box;
    Modifiers::Modifier *mod_select_cycle;
    Modifiers::Modifier *mod_select_duplicate;
    Modifiers::Modifier *mod_select_force_drag;
    Modifiers::Modifier *mod_select_in_groups;
    Modifiers::Modifier *mod_select_remove_from;
    Modifiers::Modifier *mod_select_remove_snap;
    Modifiers::Modifier *mod_select_touch_path;
};

} // namespace Inkscape::UI::Tools

#endif // INKALPHA_TOOLS_SELECT_TOOL_H