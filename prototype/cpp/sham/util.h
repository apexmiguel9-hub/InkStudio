// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * util.h — Inkscape::Util::Quantity stub used by the rect tool status text.
 */
#ifndef SHAM_UTIL_H
#define SHAM_UTIL_H

#include <cmath>
#include <string>

namespace Inkscape::Util {

// ActionAccel abstraction (select-tool keeps _acc_st_grab/scale/rotate for
// the sticky-transform key bindings; on touch they never trigger).
class ActionAccel {
public:
    explicit ActionAccel(char const * /*action_name*/) {}
    template <typename EventType> bool isTriggeredBy(EventType const &) const { return false; }
};

class Quantity {
public:
    Quantity(double value, char const * /*unit*/) : _value(value) {}

    // Formats the quantity in the requested display unit. The alpha is 1:1
    // px units (no zoom yet), so this just prints the px value.
    std::string string(std::string const & /*displayUnit*/) const {
        char buf[64];
        // avoid "-0"
        double v = std::fabs(_value) < 1e-9 ? 0.0 : _value;
        snprintf(buf, sizeof buf, "%.2f", v);
        return buf;
    }

private:
    double _value;
};

} // namespace Inkscape::Util

#endif // SHAM_UTIL_H