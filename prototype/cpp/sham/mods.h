// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * mods.h — stub of Inkscape's Modifiers::Modifier machinery.
 *
 * Both ported tools query modifiers (SELECT_* for select-tool, TRANS_* for
 * rect-tool, MOVE_* for the select key handler). With touch + no keyboard the
 * modifier state is always 0, so active() is implemented honestly against the
 * GDK masks (Ctrl/Alt/Shift) but never fires in the alpha. get_label()
 * returns the labels shown in the status text; get_and_mask() backs
 * _duplicate_drag_state() in select-tool.cpp.
 */
#ifndef SHAM_MODS_H
#define SHAM_MODS_H

#include "ink_compat.h"
#include "message.h"        // MessageContext (used by responsive_tooltip)
#include "canvas_events.h"  // Inkscape::UI::CanvasEvent
#include <string>

namespace Inkscape {

namespace UI { struct CanvasEvent; }

namespace Modifiers {

enum class Type {
    SELECT_ADD_TO,
    SELECT_FORCE_DRAG,
    SELECT_ALWAYS_BOX,
    SELECT_CYCLE,
    SELECT_DUPLICATE,
    SELECT_IN_GROUPS,
    SELECT_REMOVE_FROM,
    SELECT_REMOVE_SNAP,
    SELECT_TOUCH_PATH,
    MOVE_CONFINE,
    MOVE_INCREMENT,
    MOVE_NO_SNAPPING,
    TRANS_CONFINE,
    TRANS_OFF_CENTER,
    TRANS_INCREMENT,
    SELECT_INTERSECT,
    SELECT_DIFFERENCE,
    SELECT_SYMBOLIC,
};

// Sentinel masks (real Inkscape: Modifiers::ALWAYS/NEVER).
static constexpr unsigned ALWAYS = 0xFFFFFFFFu;
static constexpr unsigned NEVER  = 0xFFFFFFFEu;

class Modifier {
public:
    static Modifier *get(Type t) {
        // Must match the Type enum order above, 1:1.
        static Modifier instances[] = {
            {"Ctrl+Alt",       GDK_CONTROL_MASK | GDK_MOD1_MASK, 0u, false}, // SELECT_ADD_TO
            {"Alt",            GDK_MOD1_MASK,                     0u, false}, // SELECT_FORCE_DRAG
            {"Shift+Alt",      GDK_SHIFT_MASK | GDK_MOD1_MASK,    0u, false}, // SELECT_ALWAYS_BOX
            {"Alt+Shift",      GDK_MOD1_MASK | GDK_SHIFT_MASK,    0u, false}, // SELECT_CYCLE
            {"Shift+Alt",      GDK_SHIFT_MASK | GDK_MOD1_MASK,    0u, false}, // SELECT_DUPLICATE
            {"Ctrl",           GDK_CONTROL_MASK,                  0u, false}, // SELECT_IN_GROUPS
            {"Ctrl+Alt",       GDK_CONTROL_MASK | GDK_MOD1_MASK,  0u, false}, // SELECT_REMOVE_FROM
            {"Shift",          GDK_SHIFT_MASK,                    0u, false}, // SELECT_REMOVE_SNAP
            {"Alt",            GDK_MOD1_MASK,                     0u, false}, // SELECT_TOUCH_PATH
            {"Ctrl",           GDK_CONTROL_MASK,                  0u, false}, // MOVE_CONFINE
            {"Ctrl+Alt",       GDK_CONTROL_MASK | GDK_MOD1_MASK,  0u, false}, // MOVE_INCREMENT
            {"Shift",          GDK_SHIFT_MASK,                    0u, false}, // MOVE_NO_SNAPPING
            {"Ctrl",           GDK_CONTROL_MASK,                  0u, false}, // TRANS_CONFINE
            {"Alt",            GDK_MOD1_MASK,                     0u, false}, // TRANS_OFF_CENTER
            {"Ctrl+Alt",       GDK_CONTROL_MASK | GDK_MOD1_MASK,  0u, false}, // TRANS_INCREMENT
            {"Ctrl+Alt+Shift", GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_SHIFT_MASK, 0u, false}, // SELECT_INTERSECT
            {"Alt+Shift",      GDK_MOD1_MASK | GDK_SHIFT_MASK,    0u, false}, // SELECT_DIFFERENCE
            {"Ctrl+Alt+Shift", GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_SHIFT_MASK, 0u, false}, // SELECT_SYMBOLIC
        };
        return &instances[static_cast<int>(t)];
    }

    // Is this modifier held down in the given event state mask?
    bool active(unsigned state) const { return mask != 0 && (state & mask) == mask; }

    // The mask of keys that make up this modifier (for _duplicate_drag_state).
    unsigned get_and_mask() const { return mask; }

    std::string get_label() const { return std::string(label); }

    // toggle state (Inkscape "always/never" per-modifier on-screen toggles).
    bool isToggled() const { return toggled; }

private:
    Modifier(char const *l, unsigned m, unsigned /*keys*/, bool t) : label(l), mask(m), toggled(t) {}
    char const *label;
    unsigned mask;
    bool toggled;
};

// Modifier-agnostic helpers used by the tool key handlers.
inline bool keyval_is_a_modifier(unsigned /*keyval*/) { return false; }

template <typename... Ts>
inline void responsive_tooltip(MessageContext *, Inkscape::UI::CanvasEvent const &, int, Ts...) {}

// Variadic like the originalInkscape tooltip helper: rect-tool calls it with
// 3 (Type, label) pairs, select-tool with plain Types (via
// responsive_tooltip). Both compile against the variadic signature.
template <typename... Ts>
inline void responsive_tooltip_with_labels(MessageContext *, Inkscape::UI::CanvasEvent const &,
                                           int, Ts...) {}

} // namespace Modifiers
} // namespace Inkscape

#endif // SHAM_MODS_H