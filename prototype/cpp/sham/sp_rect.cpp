// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * sp_rect.cpp — SPRect implementation (alpha).
 * NodeTool helpers: nodeAt, handleAt, moveNode, moveHandle.
 */
#include "sp_rect.h"
#include "seltrans.h" // for handleRadius() = 40
#include <cmath>
#include <algorithm>

namespace {

// Distance squared
inline double dist2(const Geom::Point &a, const Geom::Point &b) {
    double dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

} // namespace

int SPRect::nodeAt(double x, double y, double tol) const {
    if (isEmpty()) return -1;
    auto c = docCorners();
    double tol2 = tol * tol;
    for (int i = 0; i < 4; ++i) {
        if (dist2({x, y}, c[i]) <= tol2) return i;
    }
    return -1;
}

int SPRect::handleAt(double x, double y, double tol) const {
    if (isEmpty()) return -1;
    auto c = docCorners();
    Geom::Point c0 = c[0], c1 = c[1], c2 = c[2], c3 = c[3];
    // Scale handles: corners + mid-edges
    Geom::Point handles[8] = {
        c0,                           // TL
        {(c0.x + c1.x) * 0.5, (c0.y + c1.y) * 0.5}, // T
        c1,                           // TR
        {(c1.x + c2.x) * 0.5, (c1.y + c2.y) * 0.5}, // R
        c2,                           // BR
        {(c2.x + c3.x) * 0.5, (c2.y + c3.y) * 0.5}, // B
        c3,                           // BL
        {(c3.x + c0.x) * 0.5, (c3.y + c0.y) * 0.5}  // L
    };
    // Rotate handles: 28px out on diagonal from corners
    const double ROT_OFF = 28.0;
    Geom::Point rotate_handles[4];
    for (int i = 0; i < 4; ++i) {
        Geom::Point next = c[(i + 1) % 4];
        Geom::Point diag = {next.x - c[i].x, next.y - c[i].y};
        double len = std::sqrt(diag.x * diag.x + diag.y * diag.y);
        if (len > 0) {
            diag.x = diag.x / len * ROT_OFF;
            diag.y = diag.y / len * ROT_OFF;
        }
        rotate_handles[i] = {c[i].x - diag.x, c[i].y - diag.y};
    }

    double tol2 = tol * tol;
    Geom::Point p(x, y);
    // Check scale handles (0-7)
    for (int i = 0; i < 8; ++i) {
        if (dist2(p, handles[i]) <= tol2) return i;
    }
    // Check rotate handles (8-11)
    for (int i = 0; i < 4; ++i) {
        if (dist2(p, rotate_handles[i]) <= tol2) return 8 + i;
    }
    return -1;
}

void SPRect::moveNode(int idx, Geom::Point pos) {
    if (idx < 0 || idx > 3 || isEmpty()) return;
    // Convert document-space pos to local space
    Geom::Point local_pos = xform.inverse().apply(pos);
    Geom::Rect l = localRect();
    double x0 = l.min()[Geom::X], y0 = l.min()[Geom::Y];
    double x1 = l.max()[Geom::X], y1 = l.max()[Geom::Y];
    // Update the specific corner
    switch (idx) {
        case 0: x0 = local_pos.x; y0 = local_pos.y; break; // TL
        case 1: x1 = local_pos.x; y0 = local_pos.y; break; // TR
        case 2: x1 = local_pos.x; y1 = local_pos.y; break; // BR
        case 3: x0 = local_pos.x; y1 = local_pos.y; break; // BL
    }
    // Ensure non-negative size (swap if needed)
    if (x1 < x0) std::swap(x0, x1);
    if (y1 < y0) std::swap(y0, y1);
    setPosition(x0, y0, x1 - x0, y1 - y0);
}

void SPRect::moveHandle(int idx, Geom::Point pos) {
    if (idx < 0 || idx > 7 || isEmpty()) return;
    // Delegate to SelTrans-style scale math: each handle scales about opposite corner/mid
    // Using the same quad logic as SelTrans::moveHandle but for a single rect.
    // We reuse the quad = docCorners() (transformed rect).
    Geom::Point q[4];
    auto c = docCorners();
    q[0] = c[0]; q[1] = c[1]; q[2] = c[2]; q[3] = c[3];

    auto mid = [](const Geom::Point &a, const Geom::Point &b) {
        return Geom::Point((a.x + b.x) * 0.5, (a.y + b.y) * 0.5);
    };

    // Anchor point for each scale handle (opposite corner/mid)
    Geom::Point anchor;
    double fx = 1.0, fy = 1.0;

    switch (idx) {
        case 0: anchor = q[2]; break; // TL -> BR
        case 1: anchor = mid(q[2], q[3]); break; // T -> mid(BR,BL)
        case 2: anchor = q[3]; break; // TR -> BL
        case 3: anchor = mid(q[3], q[0]); break; // R -> mid(BL,TL)
        case 4: anchor = q[0]; break; // BR -> TL
        case 5: anchor = mid(q[0], q[1]); break; // B -> mid(TL,TR)
        case 6: anchor = q[1]; break; // BL -> TR
        case 7: anchor = mid(q[1], q[2]); break; // L -> mid(TR,BR)
    }

    // Compute scale factors from grab position (current handle position) to new pos
    // We need the "grab position" = current handle position in doc space
    Geom::Point grab_pos;
    switch (idx) {
        case 0: grab_pos = q[0]; break;
        case 1: grab_pos = mid(q[0], q[1]); break;
        case 2: grab_pos = q[1]; break;
        case 3: grab_pos = mid(q[1], q[2]); break;
        case 4: grab_pos = q[2]; break;
        case 5: grab_pos = mid(q[2], q[3]); break;
        case 6: grab_pos = q[3]; break;
        case 7: grab_pos = mid(q[3], q[0]); break;
    }

    auto div0 = [](double n, double d) { return (d == 0.0) ? 1.0 : n / d; };
    fx = div0(pos.x - anchor.x, grab_pos.x - anchor.x);
    fy = div0(pos.y - anchor.y, grab_pos.y - anchor.y);

    // Clamp to positive (no mirroring on plain drag)
    fx = std::max(fx, 1e-3);
    fy = std::max(fy, 1e-3);

    // Apply scale about anchor to local rect
    Geom::Xform scale_xform = Geom::Xform::around(anchor, Geom::Xform::scale(fx, fy));
    // Transform the four corners
    Geom::Point new_c[4];
    for (int i = 0; i < 4; ++i) new_c[i] = scale_xform.apply(q[i]);
    // Compute new local rect from transformed corners (inverse xform)
    Geom::Point local_c[4];
    for (int i = 0; i < 4; ++i) local_c[i] = xform.inverse().apply(new_c[i]);
    double minx = std::min({local_c[0].x, local_c[1].x, local_c[2].x, local_c[3].x});
    double maxx = std::max({local_c[0].x, local_c[1].x, local_c[2].x, local_c[3].x});
    double miny = std::min({local_c[0].y, local_c[1].y, local_c[2].y, local_c[3].y});
    double maxy = std::max({local_c[0].y, local_c[1].y, local_c[2].y, local_c[3].y});
    setPosition(minx, miny, maxx - minx, maxy - miny);
}