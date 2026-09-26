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
};

} // namespace Geom

#endif // SHAM_GEOM_MIN_H