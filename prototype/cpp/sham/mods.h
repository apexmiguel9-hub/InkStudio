// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * mods.h — stub of Inkscape's Modifiers::Modifier machinery.
 *
 * rect-tool.cpp queries three modifiers (TRANS_CONFINE = make square/ratio,
 * TRANS_OFF_CENTER = draw around origin, SELECT_ADD_TO / SELECT_FORCE_DRAG).
 * With touch + no keyboard the modifier state is always 0, so active() is
 * implemented honestly against the GDK masks (Ctrl/Alt/Shift) but never fires
 * in the alpha. get_label() returns the labels shown in the status text.
 */
#ifndef SHAM_MODS_H
#define SHAM_MODS_H

#include "ink_compat.h"

namespace Inkscape {

namespace UI { struct CanvasEvent; }

namespace Modifiers {

enum class Type {
    SELECT_ADD_TO,
    SELECT_FORCE_DRAG,
    TRANS_CONFINE,
    TRANS_OFF_CENTER,
    TRANS_INCREMENT,
    SELECT_INTERSECT,
    SELECT_DIFFERENCE,
    SELECT_SYMBOLIC,
};

class Modifier {
public:
    static Modifier *get(Type t) {
        static Modifier instances[] = {
            {"Ctrl+Alt", 0, false},
            {"Alt", 0, false},
            {"Ctrl", GDK_CONTROL_MASK, false},
            {"Alt", GDK_MOD1_MASK, false},
            {"Ctrl+Alt", GDK_CONTROL_MASK | GDK_MOD1_MASK, false},
            {"Ctrl+Alt", 0, false},
            {"Alt", 0, false},
            {"Shift", 0, false},
        };
        return &instances[static_cast<int>(t)];
    }

    // Is this modifier held down in the given event state mask?
    bool active(unsigned state) const { return mask != 0 && (state & mask) == mask; }

    std::string get_label() const { return std::string(label); }

private:
    Modifier(char const *l, unsigned m, bool /*toggled*/) : label(l), mask(m) {}
    char const *label;
    unsigned mask;
};

// Modifier-agnostic helpers used by the tool key handler.
inline bool keyval_is_a_modifier(unsigned /*keyval*/) { return false; }

inline void responsive_tooltip_with_labels(
    class MessageContext * /*ctx*/, Inkscape::UI::CanvasEvent const & /*event*/, int /*timeout*/,
    Type /*m1*/, char const * /*l1*/, Type /*m2*/, char const * /*l2*/,
    Type /*m3*/, char const * /*l3*/) {
    // No keyboard in the alpha: tooltips are a no-op.
}

} // namespace Modifiers

} // namespace Inkscape

#endif // SHAM_MODS_H