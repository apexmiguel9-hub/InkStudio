// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * selection.h — stub of Inkscape::Selection.
 *
 * The rect tool drives and reads the selection on click/release. The alpha
 * has no select/click UX; the surface exists so the ported code compiles and
 * the confirm step (finishItem -> selection->set) still runs.
 */
#ifndef SHAM_SELECTION_H
#define SHAM_SELECTION_H

#include "sp_rect.h" // SPObject / SPItem

namespace Inkscape {

class Selection {
public:
    void set(SPObject *sel) { current = sel; }
    void clear() { current = nullptr; }
    bool includes(SPObject *) const { return false; }
    void toggle(SPObject *) {}
    SPItem *singleItem() const { return static_cast<SPItem *>(current); }
    void toGuides() {} // Shift+G: convert-to-guides (no-op: no guides yet)

    // signal "connection" API used by the constructor in the original code;
    // the alpha does not connect anything (see adaptation note in the port).
    struct Connection { void disconnect() {} };

private:
    SPObject *current = nullptr;
};

} // namespace Inkscape

#endif // SHAM_SELECTION_H