// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * geom_min.h — minimal 2geom subset used by rect-tool.cpp port.
 *
 * Only what the rect tool touches at runtime: Point, Rect, IntPoint, and a
 * reduced Affine. The semantics (X=0, Y=1 axis indexing, Point*Affine,
 * Affine::inverse for matrices without projective terms) mirror lib2geom;
 * the full 2geom library comes with the document/object-model phase.
 */
#ifndef SHAM_GEOM_MIN_H
#define SHAM_GEOM_MIN_H

#include <algorithm>
#include <cmath>

namespace Geom {

static constexpr int X = 0;
static constexpr int Y = 1;

struct Affine;
struct IntPoint;

struct Point {
    double x = 0.0;
    double y = 0.0;

    Point() = default;
    Point(double px, double py) : x(px), y(py) {}

    double &operator[](int axis) { return axis == X ? x : y; }
    double operator[](int axis) const { return axis == X ? x : y; }

    Point operator+(Point const &o) const { return {x + o.x, y + o.y}; }
    Point operator-(Point const &o) const { return {x - o.x, y - o.y}; }
    Point operator-() const { return {-x, -y}; }
    Point operator*(double s) const { return {x * s, y * s}; }
    friend Point operator*(double s, Point const &p) { return p * s; }
    Point operator/(double s) const { return {x / s, y / s}; }

    Point &operator+=(Point const &o) { x += o.x; y += o.y; return *this; }
    Point &operator-=(Point const &o) { x -= o.x; y -= o.y; return *this; }
    Point &operator*=(Affine const &m); // defined after Affine below

    // 2geom: Point::floor() returns an integer-valued point
    IntPoint floor() const;
};

struct IntPoint {
    int x = 0;
    int y = 0;
    IntPoint() = default;
    IntPoint(int px, int py) : x(px), y(py) {}
    IntPoint operator-(IntPoint const &o) const { return {x - o.x, y - o.y}; }
    // 2geom allows IntPoint where a Point is expected (e.g. getItemAtPoint(xyp,...)).
    operator Point() const { return Point(x, y); }
};

inline IntPoint Point::floor() const
{
    return {static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y))};
}

// L∞ norm of an integer-scaled difference (used by ToolBase::checkDragMoved)
inline int LInfty(IntPoint const &d) { return std::max(std::abs(d.x), std::abs(d.y)); }

// Reduced affine (2x3). Only the operations the rect path uses: Point*Affine
// and inverse() (translation/scale/flip-safe; projective terms NOT handled).
struct Affine {
    double v[6] = {1, 0, 0, 1, 0, 0}; // [0]=a [1]=c [2]=e / [3]=b [4]=d [5]=f (rows)

    Affine() = default;
    Affine(double a, double b, double c, double d, double e, double f)
        : v{a, c, e, b, d, f} {} // a...f => stored row-major like lib2geom

    double operator[](int i) const { return v[i]; }

    // inverse via standard 2x2 + translation, det!=0. Projective terms unused.
    Affine inverse() const {
        double det = v[0] * v[4] - v[1] * v[3];
        double a =  v[4] / det, b = -v[1] / det;
        double c = -v[3] / det, d =  v[0] / det;
        double e = -(a * v[2] + c * v[5]);
        double f = -(b * v[2] + d * v[5]);
        return {a, b, c, d, e, f};
    }
    Affine withoutTranslation() const { return {v[0], v[1], v[3], v[4], 0, 0}; }
};

inline Point operator*(Point const &p, Affine const &m) {
    return {m[0] * p[0] + m[2] * p[1] + m[4],
            m[1] * p[0] + m[3] * p[1] + m[5]};
}

inline Point &Point::operator*=(Affine const &m) {
    *this = (*this) * m;
    return *this;
}

struct Rect {
    Point _min;
    Point _max;

    Rect() = default;
    Rect(Point const &min_, Point const &max_) : _min(min_), _max(max_) {}
    Rect(Point const &origin, double w, double h)
        : _min(origin), _max(origin.x + w, origin.y + h) {}

    Point min() const { return _min; }
    Point max() const { return _max; }
    Point dimensions() const { return {_max.x - _min.x, _max.y - _min.y}; }
    double width() const { return _max.x - _min.x; }
    double height() const { return _max.y - _min.y; }
    bool empty() const { return width() <= 0 || height() <= 0; }
    bool contains(Point const &p) const {
        return p.x >= _min.x && p.x <= _max.x && p.y >= _min.y && p.y <= _max.y;
    }
    void mergeWith(Rect const &o) {
        _min.x = std::min(_min.x, o._min.x); _min.y = std::min(_min.y, o._min.y);
        _max.x = std::max(_max.x, o._max.x); _max.y = std::max(_max.y, o._max.y);
    }
};

// Rect * Affine — axis-aligned bbox of the transformed rectangle (2geom Rect
// math; used by select-tool's rubberband: (*b) * desktop->dt2doc()).
inline Rect operator*(Rect const &r, Affine const &m) {
    Point c0 = r.min() * m;
    Point c1 = Point(r.max()[Geom::X], r.min()[Geom::Y]) * m;
    Point c2 = r.max() * m;
    Point c3 = Point(r.min()[Geom::X], r.max()[Geom::Y]) * m;
    return Rect(Point(std::min({c0.x, c1.x, c2.x, c3.x}),
                      std::min({c0.y, c1.y, c2.y, c3.y})),
                Point(std::max({c0.x, c1.x, c2.x, c3.x}),
                      std::max({c0.y, c1.y, c2.y, c3.y})));
}

// Geom::OptRect used verbatim by select-tool.cpp (rubberband->getRectangle()).
using OptRect = std::optional<Rect>;

// ---------------------------------------------------------------------------
// Xform — 2geom-convention 2D affine for the alpha's item transforms.
//
//   x' = a*x + c*y + e
//   y' = b*x + d*y + f
//
// (row-major [a c e; b d f], column-vector semantics — the 2geom
// convention. ThorVG's tvg::Matrix is the TRANSPOSE (row-vector
// semantics): translation lands in e13/e23, not e31/e32; only
// renderer.cpp toTvg() adapts this form for the renderer.) The geom_min.h
// Affine above is kept untouched: the rect tool only ever hands it
// identity.
// ---------------------------------------------------------------------------
struct Xform {
    double a = 1, b = 0, c = 0, d = 1, e = 0, f = 0;

    static Xform translate(double tx, double ty) { return {1, 0, 0, 1, tx, ty}; }
    static Xform scale(double sx, double sy) { return {sx, 0, 0, sy, 0, 0}; }
    static Xform rotate(double radians) {
        double cs = std::cos(radians), sn = std::sin(radians);
        return {cs, sn, -sn, cs, 0, 0}; // CCW in y-down doc space
    }
    static Xform around(Point const &anchor, Xform const &local) {
        // anchor * local * (-anchor): applies `local` about `anchor` in doc space
        return translate(anchor.x, anchor.y) * local * translate(-anchor.x, -anchor.y);
    }

    Xform operator*(Xform const &o) const {
        // this∘o : p -> o(p) then this(p)
        return {a * o.a + c * o.b, b * o.a + d * o.b,
                a * o.c + c * o.d, b * o.c + d * o.d,
                a * o.e + c * o.f + e, b * o.e + d * o.f + f};
    }

    Point apply(Point const &p) const {
        return {a * p.x + c * p.y + e, b * p.x + d * p.y + f};
    }

    Xform inverse() const {
        double det = a * d - b * c;
        if (det == 0.0) return Xform();
        return {d / det, -b / det, -c / det, a / det,
                (c * f - d * e) / det, (b * e - a * f) / det};
    }
};

} // namespace Geom

#endif // SHAM_GEOM_MIN_H