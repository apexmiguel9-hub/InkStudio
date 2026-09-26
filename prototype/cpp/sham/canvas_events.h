// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * canvas_events.h — minimal re-implementation of Inkscape's CanvasEvent
 * hierarchy (ui/widget/events/canvas-event.h), enough for the tool handlers.
 *
 * The android touch layer synthesizes these events (button 1 = touch down,
 * motion with BUTTON1_MASK = drag, button release = finger up). Key events
 * never fire on touch but the tool's handler compiles against them.
 */
#ifndef SHAM_CANVAS_EVENTS_H
#define SHAM_CANVAS_EVENTS_H

#include "geom_min.h"
#include "ink_compat.h"
#include <type_traits>

namespace Inkscape::UI {
namespace Events {
enum EventType {
    NONE = 0,
    KEY_PRESS = 1,
    KEY_RELEASE = 2,
    BUTTON_PRESS = 4,
    BUTTON_RELEASE = 8,
    MOTION = 16,
    GRAB = 32,
    RELEASE = 64,
    ENTER = 128,
    LEAVE = 256,
    EVENT_MASK = 0xff, // grab masks only cover press/release/motion/keys
};
using EventMask = unsigned;
} // namespace Events

// Base event (all events carry a modifier mask like Inkscape's CanvasEvent).
struct CanvasEvent {
    virtual ~CanvasEvent() = default;
    unsigned modifiers = 0;
    // Modifier state *after* this event (used by the keyboard handlers).
    unsigned modifiersAfter() const { return modifiers; }
};

struct ButtonEvent : CanvasEvent {
    Geom::Point pos;
    Geom::Point orig_pos;
    unsigned button = 0;
};

struct ButtonPressEvent final : ButtonEvent {
    int num_press = 1;
};

struct ButtonReleaseEvent final : ButtonEvent {};

struct KeyEvent : CanvasEvent {
    unsigned keyval = 0;
};

struct KeyPressEvent final : KeyEvent {};
struct KeyReleaseEvent final : KeyEvent {};

struct MotionEvent final : CanvasEvent {
    Geom::Point pos;
    Geom::Point orig_pos;
};

struct EnterEvent final : CanvasEvent {};
struct LeaveEvent final : CanvasEvent {};

// Scroll wheel event (never synthesized on touch; the handler compiles).
struct ScrollEvent final : CanvasEvent {
    Geom::Point delta;
    Geom::Point pos;
};

// inspect_event: mimics Inkscape's visitor dispatcher (ui/widget/events/
// canvas-event-inspect.h) — each lambda is tried with the event types it
// accepts, walking the hierarchy (derived -> ... -> CanvasEvent); lambdas
// that accept none of them are skipped.
namespace detail {

template <typename F>
inline void dispatch(CanvasEvent const &e, F &&f) {
    if (auto const *p = dynamic_cast<ButtonPressEvent const *>(&e)) {
        if constexpr (std::is_invocable_v<F, ButtonPressEvent const &>)
            f(*p);
        else if constexpr (std::is_invocable_v<F, ButtonEvent const &>)
            f(*p);
        else if constexpr (std::is_invocable_v<F, CanvasEvent const &>)
            f(*p);
        return;
    }
    if (auto const *p = dynamic_cast<ButtonReleaseEvent const *>(&e)) {
        if constexpr (std::is_invocable_v<F, ButtonReleaseEvent const &>)
            f(*p);
        else if constexpr (std::is_invocable_v<F, ButtonEvent const &>)
            f(*p);
        else if constexpr (std::is_invocable_v<F, CanvasEvent const &>)
            f(*p);
        return;
    }
    if (auto const *p = dynamic_cast<MotionEvent const *>(&e)) {
        if constexpr (std::is_invocable_v<F, MotionEvent const &>)
            f(*p);
        else if constexpr (std::is_invocable_v<F, CanvasEvent const &>)
            f(*p);
        return;
    }
    if (auto const *p = dynamic_cast<KeyPressEvent const *>(&e)) {
        if constexpr (std::is_invocable_v<F, KeyPressEvent const &>)
            f(*p);
        else if constexpr (std::is_invocable_v<F, KeyEvent const &>)
            f(*p);
        else if constexpr (std::is_invocable_v<F, CanvasEvent const &>)
            f(*p);
        return;
    }
    if (auto const *p = dynamic_cast<KeyReleaseEvent const *>(&e)) {
        if constexpr (std::is_invocable_v<F, KeyReleaseEvent const &>)
            f(*p);
        else if constexpr (std::is_invocable_v<F, KeyEvent const &>)
            f(*p);
        else if constexpr (std::is_invocable_v<F, CanvasEvent const &>)
            f(*p);
        return;
    }
    if (auto const *p = dynamic_cast<ScrollEvent const *>(&e)) {
        if constexpr (std::is_invocable_v<F, ScrollEvent const &>)
            f(*p);
        else if constexpr (std::is_invocable_v<F, CanvasEvent const &>)
            f(*p);
        return;
    }
    if (auto const *p = dynamic_cast<EnterEvent const *>(&e)) {
        if constexpr (std::is_invocable_v<F, EnterEvent const &>)
            f(*p);
        else if constexpr (std::is_invocable_v<F, CanvasEvent const &>)
            f(*p);
        return;
    }
    if (auto const *p = dynamic_cast<LeaveEvent const *>(&e)) {
        if constexpr (std::is_invocable_v<F, LeaveEvent const &>)
            f(*p);
        else if constexpr (std::is_invocable_v<F, CanvasEvent const &>)
            f(*p);
        return;
    }
    // bare canvas event: only a fallback lambda can consume it.
    if constexpr (std::is_invocable_v<F, CanvasEvent const &>)
        f(e);
}
} // namespace detail

template <typename... Fs>
void inspect_event(CanvasEvent const &e, Fs &&...fs) {
    (detail::dispatch(e, std::forward<Fs>(fs)), ...);
}

} // namespace Inkscape::UI

// Helpers referenced unqualified by the tool key/motion handlers
// (the originals live in Inkscape's context-fns / tool-base headers).
inline unsigned get_latin_keyval(Inkscape::UI::KeyEvent const &event) { return event.keyval; }

inline bool mod_shift_only(Inkscape::UI::CanvasEvent const &event) {
    return (event.modifiers & GDK_SHIFT_MASK) && !(event.modifiers & (GDK_CONTROL_MASK | GDK_MOD1_MASK));
}

inline bool mod_ctrl_only(Inkscape::UI::CanvasEvent const &event) {
    return (event.modifiers & GDK_CONTROL_MASK) && !(event.modifiers & (GDK_SHIFT_MASK | GDK_MOD1_MASK));
}

// Single-key modifier tests referenced by select-tool.cpp.
inline bool mod_ctrl(Inkscape::UI::CanvasEvent const &event) {
    return (event.modifiers & GDK_CONTROL_MASK) != 0;
}

inline bool mod_shift(Inkscape::UI::CanvasEvent const &event) {
    return (event.modifiers & GDK_SHIFT_MASK) != 0;
}

inline bool mod_alt(Inkscape::UI::CanvasEvent const &event) {
    return (event.modifiers & GDK_MOD1_MASK) != 0;
}

#endif // SHAM_CANVAS_EVENTS_H