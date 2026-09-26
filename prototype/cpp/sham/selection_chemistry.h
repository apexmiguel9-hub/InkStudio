// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * selection_chemistry.h — stubs for selection-chemistry.h (keyboard helpers).
 *
 * Tab/shift-Tab item cycling and Ctrl+A select-all are keyboard-only in
 * Inkscape; on touch they never fire. The stubs keep the select-tool port
 * compiling unchanged.
 */
#ifndef SHAM_SELECTION_CHEMISTRY_H
#define SHAM_SELECTION_CHEMISTRY_H

class SPDesktop;

namespace Inkscape {

// Tab / Shift+Tab: cycle selection through the items in the document.
void sp_selection_item_next(SPDesktop *) {}
void sp_selection_item_prev(SPDesktop *) {}

// Ctrl+A: select everything.
void sp_edit_select_all(SPDesktop *) {}

} // namespace Inkscape

#endif // SHAM_SELECTION_CHEMISTRY_H