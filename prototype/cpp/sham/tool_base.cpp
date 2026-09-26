// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * tool_base.cpp — verbatim implementations of the ToolBase drag helpers,
 * copied from Inkscape ui/tools/tool-base.cpp (saveDragOrigin,
 * checkDragMoved, setup_for_drag_start).
 */
#include "tool_base.h"

namespace Inkscape::UI::Tools {

/**
 * Called by the tool when the mouse/touch is pressed.
 */
void ToolBase::setup_for_drag_start(ButtonPressEvent const &event)
{
    // Behave as though ON_KEYUP of button1 (touch down = button 1).
    if (event.button == 1) {
        _button1on = true;
    }
    saveDragOrigin(event.pos);
}

/**
 * Records the point where the drag started.
 */
void ToolBase::saveDragOrigin(Geom::Point const &pos)
{
    xyp = pos.floor();
    within_tolerance = true;
}

/**
 * Analyse the current position and return true once it has moved farther than
 * tolerance from the drag origin (indicating they intend to move the object,
 * not click).
 */
bool ToolBase::checkDragMoved(Geom::Point const &pos)
{
    if (within_tolerance) {
        if (Geom::LInfty(pos.floor() - xyp) < tolerance) {
            // Do not drag if within tolerance from origin.
            return false;
        }
        // Mark drag as started.
        within_tolerance = false;
    }
    // Always return true once the drag has started.
    return true;
}

} // namespace Inkscape::UI::Tools