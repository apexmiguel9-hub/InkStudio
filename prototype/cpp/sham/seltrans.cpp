// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * seltrans.cpp — alpha SelTrans engine (replaces Inkscape's knot layer).
 *
 * See seltrans.h for the design. All math is document-space 2geom-convention
 * (Xform): the renderer maps the result onto tvg::Matrix per frame.
 */
#include "seltrans.h"
#include "desktop.h"

#include <algorithm>
#include <cmath>

static Inkscape::SelTrans *g_active_seltrans = nullptr;

// Division that stays finite near zero (scaling two degenerate axes).
static double div0(double num, double den)
{
    if (std::fabs(den) < 1e-6) {
        return 1.0;
    }
    return num / den;
}

namespace Inkscape {

SelTrans *active_seltrans() { return g_active_seltrans; }

SelTrans::SelTrans(SPDesktop *desktop)
    : _desktop(desktop)
{
    g_active_seltrans = this;
}

SelTrans::~SelTrans()
{
    if (g_active_seltrans == this) {
        g_active_seltrans = nullptr;
    }
}

bool SelTrans::isEmpty() const
{
    return _desktop->getSelection()->isEmpty();
}

std::vector<SPRect *> SelTrans::selectedRects() const
{
    std::vector<SPRect *> rects;
    for (SPItem *item : _desktop->getSelection()->items()) {
        if (auto *r = dynamic_cast<SPRect *>(item)) {
            if (!r->isEmpty()) {
                rects.push_back(r);
            }
        }
    }
    return rects;
}

Geom::OptRect SelTrans::bbox() const
{
    Geom::OptRect box;
    for (SPRect *r : selectedRects()) {
        Geom::Rect rb = r->docBBox();
        if (!box) {
            box = rb;
        } else {
            (*box).mergeWith(rb);
        }
    }
    return box;
}

// The selection's *transformed* outline, as the four doc-space corners of the
// item's local box under its current xform (TL, TR, BR, BL). The overlay (cue
// + handles) is drawn from this quad so the box rides the rotated corners
// instead of the axis-aligned bbox — Inkscape draws its mid-drag outline the
// same way (_bbox->corner(i) * affine, seltrans.cpp:407). Multi-selections
// fall back to the union AABB (axis-aligned box around everything).
std::array<Geom::Point, 4> SelTrans::selectionQuad() const
{
    auto rects = selectedRects();
    if (rects.size() == 1) {
        return rects[0]->docCorners();
    }
    if (auto box = bbox()) {
        return {box->min(),
                Geom::Point(box->max()[Geom::X], box->min()[Geom::Y]),
                box->max(),
                Geom::Point(box->min()[Geom::X], box->max()[Geom::Y])};
    }
    return {Geom::Point(), Geom::Point(), Geom::Point(), Geom::Point()};
}

Geom::Point SelTrans::center() const
{
    if (_center_explicit) {
        return _center;
    }
    if (auto box = bbox()) {
        return {box->min().x + box->width() * 0.5, box->min().y + box->height() * 0.5};
    }
    return {};
}

void SelTrans::snapshot()
{
    _g.start_xforms.clear();
    for (SPRect *r : selectedRects()) {
        _g.start_xforms.emplace_back(r, r->xform);
    }
    if (auto box = bbox()) {
        _g.bbox0 = *box;
    } else {
        _g.bbox0 = Geom::Rect({}, 0, 0);
    }
    _g.quad0 = selectionQuad();
    _g.center0 = center();
}

// Seed the continuous-rotation bookkeeping from the grab point. Without this
// a rotate drag that crosses the bbox center would flip the angle by PI and
// the box would "turn over like a page"; accumulating the smallest per-move
// angle keeps the rotation continuous through the center.
void SelTrans::initRotateState()
{
    _g.last_ang = std::atan2(_g.grab_pos.y - _g.center0.y,
                             _g.grab_pos.x - _g.center0.x);
    _g.rot_accum = 0.0;
}

// ---- select-tool protocol ----------------------------------------------

void SelTrans::increaseState()
{
    // scale <-> rotate (toggled by TAP on selected item; align omitted: no
    // /tools/select/align pref). Rotation was briefly disabled while the
    // scale path was verified end-to-end; both are now pixel-verified.
    _state = (_state == STATE_SCALE) ? STATE_ROTATE : STATE_SCALE;
    if (!_center_explicit) {
        _center = Geom::Point{}; // recompute from the selection bbox
    }
}

void SelTrans::resetState(State state)
{
    _state = state;
    if (!_center_explicit) {
        _center = Geom::Point{};
    }
}

void SelTrans::setCenter(Geom::Point const &p)
{
    _center = p;
    _center_explicit = true;
}

void SelTrans::grab(Geom::Point const &p, double, double, bool, bool translating)
{
    if (isEmpty()) {
        return;
    }
    _translating = translating;
    _grabbed = true;
    _g.grab_pos = p;
    snapshot();
    initRotateState();
}

bool SelTrans::moveTo(Geom::Point const &xy, unsigned)
{
    if (!_grabbed || !_translating) {
        return false;
    }
    Geom::Point delta = xy - _g.grab_pos;
    Geom::Xform t = Geom::Xform::translate(delta.x, delta.y);
    for (auto &[rect, x0] : _g.start_xforms) {
        rect->xform = t * x0;
    }
    return true;
}

void SelTrans::ungrab()
{
    _grabbed = false;
    _translating = false;
    _g.start_xforms.clear();
}

// ---- touch handle engine -----------------------------------------------

double SelTrans::handleRadius()
{
    // ~15dp on a 2.6x density device ≈ 40px; tunable via the real prefs.
    return Inkscape::Preferences::get()->getIntLimited("/options/touch/handle_radius", 40, 10, 200);
}

std::vector<SelTrans::HandlePos> SelTrans::handlePositions() const
{
    std::vector<HandlePos> pos;
    if (!visible()) {
        return pos;
    }
    auto box = bbox();
    if (!box) {
        return pos;
    }
    double mx = box->min().x + box->width() * 0.5;
    double my = box->min().y + box->height() * 0.5;
    Geom::Point c(mx, my);
    if (_center_explicit) {
        c = _center;
    }

    // Handles ride the *transformed* selection box (a rotated quad) rather
    // than its axis-aligned bbox, so they stay glued to the rotated corners
    // while the item spins (Inkscape's _bbox->corner(i) * affine outline).
    auto q = selectionQuad();
    auto mid = [](Geom::Point const &a, Geom::Point const &b) {
        return Geom::Point((a.x + b.x) * 0.5, (a.y + b.y) * 0.5);
    };

    if (_state == STATE_SCALE) {
        pos.push_back({Handle::TL, q[0]});
        pos.push_back({Handle::T,  mid(q[0], q[1])});
        pos.push_back({Handle::TR, q[1]});
        pos.push_back({Handle::R,  mid(q[1], q[2])});
        pos.push_back({Handle::BR, q[2]});
        pos.push_back({Handle::B,  mid(q[2], q[3])});
        pos.push_back({Handle::BL, q[3]});
        pos.push_back({Handle::L,  mid(q[3], q[0])});
    } else { // STATE_ROTATE: corners pushed out along the diagonal
        Geom::Point const corners[4] = {q[0], q[1], q[2], q[3]};
        Handle const ids[4] = {Handle::ROT_TL, Handle::ROT_TR,
                               Handle::ROT_BR, Handle::ROT_BL};
        for (int i = 0; i < 4; ++i) {
            Geom::Point dir = corners[i] - c;
            double len = std::hypot(dir.x, dir.y);
            if (len < 1e-6) {
                continue;
            }
            dir = dir * (1.0 / len);
            pos.push_back({ids[i], corners[i] + dir * 28.0});
        }
        pos.push_back({Handle::CENTER, c});
    }
    return pos;
}

bool SelTrans::tryGrabHandle(Geom::Point const &p)
{
    if (!visible() || isEmpty()) {
        return false;
    }
    double rad = handleRadius();
    Handle best = Handle::NONE;
    Geom::Point best_pos;
    double best_d = rad * rad; // within the touch radius
    for (auto const &hp : handlePositions()) {
        double dx = hp.pos.x - p.x;
        double dy = hp.pos.y - p.y;
        double d2 = dx * dx + dy * dy;
        if (d2 <= best_d) {
            best_d = d2;
            best = hp.id;
            best_pos = hp.pos;
        }
    }
    if (best == Handle::NONE) {
        return false;
    }
    _drag_handle = best;
    // Anchor the drag to the handle's exact center, not the raw finger point:
    // on touch the finger can land up to handleRadius() away from the visual
    // handle, and using the finger point would leave the resized box parked
    // off-center from the finger for the whole drag.
    _g.grab_pos = best_pos;
    snapshot();
    initRotateState();
    return true;
}

bool SelTrans::moveHandle(Geom::Point const &p)
{
    if (_drag_handle == Handle::NONE || isEmpty()) {
        return false;
    }

    // center-drag in rotate mode just moves the pivot
    if (_drag_handle == Handle::CENTER) {
        _center = p;
        _center_explicit = true;
        return true;
    }

    if (_g.start_xforms.empty()) {
        return false;
    }

    bool rotate = false;
    Geom::Point anchor;
    double fx = 1.0, fy = 1.0;

    if (_state == STATE_ROTATE) {
        rotate = true;
    } else {
        // scale/stretch: each handle scales about the opposite edge/corner of
        // the snapshot *quad* (the transformed box). Using the rotated frame
        // keeps the anchor glued to the box while scaling a rotated rect; for
        // an untransformed selection the quad equals the bbox, so the math is
        // identical to the old AABB version.
        auto const &qd = _g.quad0;
        Geom::Point const qt = qd[0], qtr = qd[1];
        Geom::Point const qbr = qd[2], qbl = qd[3];
        auto const mid = [](Geom::Point const &a, Geom::Point const &b) {
            return Geom::Point((a.x + b.x) * 0.5, (a.y + b.y) * 0.5);
        };
        Geom::Point s = _g.grab_pos;
        switch (_drag_handle) {
            case Handle::TL:
                anchor = qbr;
                fx = div0(p.x - qbr.x, s.x - qbr.x);
                fy = div0(p.y - qbr.y, s.y - qbr.y);
                break;
            case Handle::T:
                anchor = mid(qbr, qbl);
                fx = 1.0;
                fy = div0(p.y - anchor.y, s.y - anchor.y);
                break;
            case Handle::TR:
                anchor = qbl;
                fx = div0(p.x - qbl.x, s.x - qbl.x);
                fy = div0(p.y - qbl.y, s.y - qbl.y);
                break;
            case Handle::R:
                anchor = mid(qbl, qt);
                fx = div0(p.x - anchor.x, s.x - anchor.x);
                fy = 1.0;
                break;
            case Handle::BR:
                anchor = qt;
                fx = div0(p.x - qt.x, s.x - qt.x);
                fy = div0(p.y - qt.y, s.y - qt.y);
                break;
            case Handle::B:
                anchor = mid(qt, qtr);
                fx = 1.0;
                fy = div0(p.y - anchor.y, s.y - anchor.y);
                break;
            case Handle::BL:
                anchor = qtr;
                fx = div0(p.x - qtr.x, s.x - qtr.x);
                fy = div0(p.y - qtr.y, s.y - qtr.y);
                break;
            case Handle::L:
                anchor = mid(qtr, qbr);
                fx = div0(p.x - anchor.x, s.x - anchor.x);
                fy = 1.0;
                break;
            default:
                return false;
        }
    }

    Geom::Xform op;
    if (rotate) {
        // Continuous rotation: accumulate the smallest per-move angle instead
        // of an absolute atan2 against the grab point, so crossing the bbox
        // center spins smoothly instead of flipping the box by PI.
        Geom::Point c = _g.center0;
        double ang = std::atan2(p.y - c.y, p.x - c.x);
        double d = ang - _g.last_ang;
        if (d > M_PI) d -= 2.0 * M_PI;
        if (d < -M_PI) d += 2.0 * M_PI;
        _g.rot_accum += d;
        _g.last_ang = ang;
        op = Geom::Xform::around(c, Geom::Xform::rotate(_g.rot_accum));
    } else {
        // No mirrors on plain touch drag: clamp the scale factors positive so
        // dragging a handle past its opposite anchor never flips the box
        // ("se abre como una hoja"). Flipping stays a keyboard-modifier
        // affair in Inkscape.
        fx = std::max(fx, 1e-3);
        fy = std::max(fy, 1e-3);
        op = Geom::Xform::around(anchor, Geom::Xform::scale(fx, fy));
    }

    for (auto &[rect, x0] : _g.start_xforms) {
        rect->xform = op * x0;
    }
    return true;
}

void SelTrans::releaseHandle()
{
    if (_drag_handle != Handle::NONE) {
        _drag_handle = Handle::NONE;
        _g.start_xforms.clear();
    }
}

} // namespace Inkscape