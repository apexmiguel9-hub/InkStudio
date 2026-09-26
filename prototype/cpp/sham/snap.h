// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * snap.h — snapping stubs + declarations for context-fns.cpp port.
 *
 * rect-tool relies on snap_rectangular_box() (copied in snap.cpp) for the
 * drag math: Ctrl = integer ratio / golden-ratio confine, Alt = draw around
 * the starting point. The snap *engine* is stubbed (every SnappedPoint
 * reports "not snapped"), so the pure geometry path is exercised — exactly
 * what the alpha needs. Real snapping arrives with the document phase.
 */
#ifndef SHAM_SNAP_H
#define SHAM_SNAP_H

#include "geom_min.h"
#include "ink_compat.h"
#include <limits>

class SPDesktop;
class SPItem;

namespace Inkscape {

namespace UI { namespace Tools { class ToolBase; } }
class MessageContext;

enum SnapsourceType {
    SNAPSOURCE_UNDEFINED,
    SNAPSOURCE_NODE_HANDLE,
};

class SnapCandidatePoint {
public:
    SnapCandidatePoint(Geom::Point const &point, SnapsourceType) : point(point) {}
    Geom::Point const &getPoint() const { return point; }

private:
    Geom::Point point;
};

namespace Snapper {
struct SnapConstraint {
    explicit SnapConstraint(Geom::Point const &vec) : vector(vec) {}
    Geom::Point vector;
};
} // namespace Snapper

class SnappedPoint {
public:
    SnappedPoint() = default;
    SnappedPoint(Geom::Point const &p, bool s) : point(p), snapped(s) {}
    bool getSnapped() const { return snapped; }
    double getSnapDistance() const { return snapped ? 0.0 : std::numeric_limits<double>::max(); }
    Geom::Point getPoint() const { return point; }

private:
    Geom::Point point;
    bool snapped = false;
};

// Stub snap manager (all snaps report "not snapped" -> pure geometry path).
class SnapManager {
public:
    void setup(SPDesktop const * /*desktop*/, bool /*enabled*/ = false, SPItem * /*item*/ = nullptr) {}
    void unSetup() {}
    SnappedPoint constrainedSnap(SnapCandidatePoint const &p, Snapper::SnapConstraint const &) {
        return {p.getPoint(), false};
    }
    SnappedPoint freeSnap(SnapCandidatePoint const &p) { return {p.getPoint(), false}; }
    SnappedPoint freeSnapReturnByRef(Geom::Point &p, SnapsourceType) { return {p, false}; }
    void preSnap(SnapCandidatePoint const &) {}
};

// ---- context-fns.h surface (stubs) ------------------------------------
SPItem *sp_event_context_find_item(SPDesktop const *, Geom::Point const &, bool, bool);
bool sp_event_context_knot_mouseover();
bool have_viable_layer(SPDesktop const *, MessageContext *);
void sp_event_context_read(UI::Tools::ToolBase *, char const *);

// The rect tool drag math — ported verbatim in snap.cpp.
Geom::Rect snap_rectangular_box(SPDesktop const *desktop, SPItem *item,
                                Geom::Point const &pt, Geom::Point const &center, int state);

} // namespace Inkscape

#endif // SHAM_SNAP_H