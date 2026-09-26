// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * snap.cpp — port of Inkscape's snap_rectangular_box (helper/context-fns.cpp)
 * plus the context-fns stubs the rect tool needs.
 *
 * The function body below is copied VERBATIM from the Inkscape sources
 * (context-fns.cpp, snap_rectangular_box) with only the service stubs
 * feeding it (SnapManager no-op, Modifiers reading the event state mask).
 * With the snap engine stubbed (nothing "snaps"), this is the plain
 * drag-geometry: Ctrl-confine (integer/golden ratio) and Alt-off-center.
 */
#include "snap.h"

#include "desktop.h"
#include "message.h"
#include "mods.h"
#include "prefs.h"
#include "tool_base.h" // full ToolBase definition (prefsPath(), set())

#include <cmath>

static const double midpt_1_goldenratio = (1 + goldenratio) / 2;
static const double midpt_goldenratio_2 = (goldenratio + 2) / 2;

namespace Inkscape {

// ---------------------------------------------------------------------
// context-fns.h stubs
// ---------------------------------------------------------------------

SPItem *sp_event_context_find_item(SPDesktop const *desktop, Geom::Point const &p, bool select_under, bool into_groups)
{
    // Real hit-testing: topmost item whose transformed geometry contains p.
    // select_under picks the bottom-most of the stack (used by alt+click and
    // the node tool; unreachable on touch but implemented anyway).
    if (select_under) {
        auto items = desktop->getItemsAtPoints({p}, false, false);
        return desktop->getItemFromListAtPointBottom(items, p);
    }
    return desktop->getItemAtPoint(p, into_groups, nullptr);
}

bool sp_event_context_knot_mouseover()
{
    // Alpha: no knot (control-point) hover.
    return false;
}

bool have_viable_layer(SPDesktop const *, MessageContext *)
{
    // Alpha: no layer model; drawing is always allowed.
    return true;
}

void sp_event_context_read(UI::Tools::ToolBase *tool, char const *key)
{
    // Alpha pref bridge: /tools/<tool>/<key> -> ToolBase::set(Entry). The
    // same shape as Inkscape's context-fns.cpp (reads the pref, hands the
    // entry to the tool). String keys ("show"/"transform") and double keys
    // (rx/ry) both flow through; unknown keys are no-ops in the tools' set().
    auto *prefs = Inkscape::Preferences::get();
    std::string path = tool->prefsPath() + "/" + key;
    tool->set(Inkscape::Preferences::Entry(std::string(key), prefs->getString(path)));
}

// ---------------------------------------------------------------------
// snap_rectangular_box — verbatim from context-fns.cpp
// ---------------------------------------------------------------------

Geom::Rect snap_rectangular_box(SPDesktop const *desktop, SPItem *item,
                                Geom::Point const &pt, Geom::Point const &center, int state)
{
    desktop->getSnapIndicator()->remove_snaptarget();
    Geom::Point p[2];

    auto confine = Modifiers::Modifier::get(Modifiers::Type::TRANS_CONFINE)->active(state);
    auto off_center = Modifiers::Modifier::get(Modifiers::Type::TRANS_OFF_CENTER)->active(state);
    /* Check if Alt key is pressed */
    auto rect_edge = Modifiers::Modifier::get(Modifiers::Type::TRANS_INCREMENT)->active(state);

    auto &m = desktop->getNamedView()->snap_manager;
    m.setup(desktop, false, item);
    Inkscape::SnappedPoint snappoint;

    if (confine) {
        /* We are constrained to producing integer-ratio rectangles */

        /* Vector from the centre of the box to the point we are dragging to */
        Geom::Point delta = pt - center;

        if (!rect_edge) {
            /* Round it so that we have an integer-ratio (or golden ratio) box */
            if (fabs(delta[Geom::X]) > fabs(delta[Geom::Y]) && (delta[Geom::Y] != 0.0)) {
                double ratio = delta[Geom::X] / delta[Geom::Y];
                double ratioabs = fabs (ratio);
                double sign = (ratio < 0 ? -1 : 1);
                if (midpt_1_goldenratio < ratioabs && ratioabs < midpt_goldenratio_2) {
                    delta[Geom::X] = sign * goldenratio * delta[Geom::Y];
                } else {
                    delta[Geom::X] = floor(ratio + 0.5) * delta[Geom::Y];
                }
            } else if (delta[Geom::X] != 0.0) {
                double ratio = delta[Geom::Y] / delta[Geom::X];
                double ratioabs = fabs(ratio);
                double sign = (ratio < 0 ? -1 : 1);
                if (midpt_1_goldenratio < ratioabs && ratioabs < midpt_goldenratio_2) {
                    delta[Geom::Y] = sign * goldenratio * delta[Geom::X];
                } else {
                    delta[Geom::Y] = floor(delta[Geom::Y] / delta[Geom::X] + 0.5) * delta[Geom::X];
                }
            }
        } else {
            /* Since Alt+Ctrl is pressed we make mouse pointer lie on square with origin as one corner */
            double ratio = delta[Geom::Y] / delta[Geom::X];
            double sign = (ratio < 0 ? -1 : 1);
            if (fabs(delta[Geom::X]) > fabs(delta[Geom::Y]) && (delta[Geom::Y] != 0.0)) {
                delta[Geom::Y] = sign * delta[Geom::X];
            } else if (delta[Geom::X] != 0.0) {
                delta[Geom::X] = sign * delta[Geom::Y];
            }
        }

        /* p[1] is the dragged point with the integer-ratio constraint */
        p[1] = center + delta;

        if (off_center) {

            // Our origin is the centre point rather than the corner point;
            // this means that corner-point movements are bound to each other.

            /* p[0] is the opposite corner of our box */
            p[0] = center - delta;

            Inkscape::SnappedPoint s[2];

            /* Try to snap p[0] (the opposite corner) along the constraint vector */
            s[0] = m.constrainedSnap(Inkscape::SnapCandidatePoint(p[0], Inkscape::SNAPSOURCE_NODE_HANDLE),
                                     Inkscape::Snapper::SnapConstraint(p[0] - p[1]));

            /* Try to snap p[1] (the dragged corner) along the constraint vector */
            s[1] = m.constrainedSnap(Inkscape::SnapCandidatePoint(p[1], Inkscape::SNAPSOURCE_NODE_HANDLE),
                                     Inkscape::Snapper::SnapConstraint(p[1] - p[0]));

            /* Choose the best snap and update points accordingly */
            if (s[0].getSnapDistance() < s[1].getSnapDistance()) {
                if (s[0].getSnapped()) {
                    p[0] = s[0].getPoint();
                    p[1] = 2 * center - s[0].getPoint();
                    snappoint = s[0];
                }
            } else {
                if (s[1].getSnapped()) {
                    p[0] = 2 * center - s[1].getPoint();
                    p[1] = s[1].getPoint();
                    snappoint = s[1];
                }
            }
        } else {

            /* Our origin is the opposite corner.  Snap the drag point along the constraint vector */
            p[0] = center;
            snappoint = m.constrainedSnap(Inkscape::SnapCandidatePoint(p[1], Inkscape::SNAPSOURCE_NODE_HANDLE),
                                          Inkscape::Snapper::SnapConstraint(p[1] - p[0]));
            if (snappoint.getSnapped()) {
                p[1] = snappoint.getPoint();
            }
        }

    } else if (off_center) {
        // Our origin is the centre point rather than the corner point;
        // this means that corner-point movements are bound to each other.

        p[1] = pt;
        p[0] = 2 * center - p[1];

        Inkscape::SnappedPoint s[2];

        s[0] = m.freeSnap(Inkscape::SnapCandidatePoint(p[0], Inkscape::SNAPSOURCE_NODE_HANDLE));
        s[1] = m.freeSnap(Inkscape::SnapCandidatePoint(p[1], Inkscape::SNAPSOURCE_NODE_HANDLE));

        if (s[0].getSnapDistance() < s[1].getSnapDistance()) {
            if (s[0].getSnapped()) {
                p[0] = s[0].getPoint();
                p[1] = 2 * center - s[0].getPoint();
                snappoint = s[0];
            }
        } else {
            if (s[1].getSnapped()) {
                p[0] = 2 * center - s[1].getPoint();
                p[1] = s[1].getPoint();
                snappoint = s[1];
            }
        }

    } else {

        /* There's no constraint on the corner point, so just snap it to anything */
        p[0] = center;
        p[1] = pt;
        snappoint = m.freeSnap(Inkscape::SnapCandidatePoint(pt, Inkscape::SNAPSOURCE_NODE_HANDLE));
        if (snappoint.getSnapped()) {
            p[1] = snappoint.getPoint();
        }
    }

    if (snappoint.getSnapped()) {
        desktop->getSnapIndicator()->set_new_snaptarget(snappoint);
    }

    p[0] *= desktop->dt2doc();
    p[1] *= desktop->dt2doc();

    m.unSetup();

    return Geom::Rect(Geom::Point(MIN(p[0][Geom::X], p[1][Geom::X]), MIN(p[0][Geom::Y], p[1][Geom::Y])),
                      Geom::Point(MAX(p[0][Geom::X], p[1][Geom::X]), MAX(p[0][Geom::Y], p[1][Geom::Y])));
}

} // namespace Inkscape