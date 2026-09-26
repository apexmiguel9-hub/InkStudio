// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * actions_tools.h — stub of actions/actions-tools.h.
 *
 * Inkscape's double-tap on an object switches to the node tool via
 * set_active_tool(). The alpha's toolbar is the explicit switching UI, so
 * this is a no-op (a double-tap on a shape just re-selected nothing yet).
 */
#ifndef SHAM_ACTIONS_TOOLS_H
#define SHAM_ACTIONS_TOOLS_H

#include "geom_min.h"
#include "sp_rect.h" // SPItem

class SPDesktop;

namespace Inkscape {

inline void set_active_tool(SPDesktop *, SPItem *, Geom::Point const &) {}

} // namespace Inkscape

#endif // SHAM_ACTIONS_TOOLS_H