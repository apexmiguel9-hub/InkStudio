// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * ink_compat.h — compat layer for porting Inkscape tool code to the native
 * Android prototype ("InkAlpha").
 *
 * This file provides the *macros and tiny aliases* that Inkscape tool sources
 * reference implicitly (i18n, GDK masks, golden ratio constants, CLAMP...).
 * Bigger service stubs live in the other sham/ headers.
 */
#ifndef SHAM_INK_COMPAT_H
#define SHAM_INK_COMPAT_H

#include <algorithm>
#include <cmath>
#include <string>

// i18n: alpha has no translations; _() is identity.
#ifndef _
#define _(s) (s)
#endif

// glib typedefs/macros used by the ported code
using gdouble = double;

#ifndef CLAMP
#define CLAMP(x, lo, hi) std::clamp((x), static_cast<double>(lo), static_cast<double>(hi))
#endif
#ifndef MIN
#define MIN(a, b) std::min((a), (b))
#endif
#ifndef MAX
#define MAX(a, b) std::max((a), (b))
#endif

// gettext context+icon macros referenced by finishItem()
#define RC_(ctx, text) (text)
#define INKSCAPE_ICON(name) (name)

// GDK modifier masks (from gdktypes.h) — the tool's event logic tests these.
// Touch events in the alpha always pass modifiers=0 (no keyboard), so only
// the constants are needed for the code to compile; the keyboard-less path
// never sets them.
#define GDK_SHIFT_MASK    (1 << 0)
#define GDK_LOCK_MASK     (1 << 1)
#define GDK_CONTROL_MASK  (1 << 2)
#define GDK_MOD1_MASK     (1 << 3)
#define GDK_MOD2_MASK     (1 << 4)
#define GDK_MOD3_MASK     (1 << 5)
#define GDK_MOD4_MASK     (1 << 6)
#define GDK_MOD5_MASK     (1 << 7)
#define GDK_BUTTON1_MASK  (1 << 8)
#define GDK_BUTTON2_MASK  (1 << 9)
#define GDK_BUTTON3_MASK  (1 << 10)

// GDK keysyms referenced by rect-tool.cpp key handler
#define GDK_KEY_Escape     0xff1b
#define GDK_KEY_space      0x20
#define GDK_KEY_g          0x67
#define GDK_KEY_G          0x47
#define GDK_KEY_Delete     0xffff
#define GDK_KEY_KP_Delete  0xff9f
#define GDK_KEY_BackSpace  0xff08

// Inkscape: 2geom/geometry.h — golden ratio used by the constrain math.
static const double goldenratio = 1.6180339887498948482;

// Rect-tool references "Glib::ustring" for status text formatting; alpha uses
// std::string, so we provide the alias the ported code compiles against.
namespace Glib {
using ustring = std::string;
} // namespace Glib

#endif // SHAM_INK_COMPAT_H