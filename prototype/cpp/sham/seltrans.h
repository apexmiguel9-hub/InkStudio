// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * seltrans.h — Inkscape::SelTrans port: the selection-transform engine.
 *
 * In Inkscape this is a big class that builds per-item knotholders on the
 * canvas and answers the tool's grab/moveTo/ungrab protocol. In the alpha
 * the whole "8 knots + rotate handles" layer is replaced by a small engine
 * with the SAME API surface the select-tool.cpp port calls (grab, moveTo,
 * ungrab, increaseState, resetState, is_stkey=false, getSelCue stub), plus a
 * touch-handle engine the renderer drives directly:
 *
 *   Renderer::touch (DOWN) → tryGrabHandle(p)      (handle above item hit)
 *   Renderer::touch (MOVE) → moveHandle(p)
 *   Renderer::touch (UP)   → releaseHandle()
 *
 * and handlePositions()/state() for the renderer overlay.
 *
 * Math (document space, y down):
 *   move    : T(delta) ∘ xform
 *   scale   : T(anchor)·S(fx,fy)·T(-anchor) ∘ xform
 *   rotate  : T(center)·R(angle)·T(-center) ∘ xform
 */
#ifndef SHAM_SELTRANS_H
#define SHAM_SELTRANS_H

#include "geom_min.h"
#include "sp_rect.h"
#include "selection.h"
#include "prefs.h"
#include <vector>

class SPDesktop;

namespace Inkscape {

class SelTrans {
public:
    // State machine: scale handles ↔ rotate handles (toggle per click).
    enum State { STATE_SCALE, STATE_ROTATE, STATE_ALIGN };
    enum Show { SHOW_CONTENT, SHOW_OUTLINE }; // /tools/select/show pref
    enum class StickyTransform { Grab, Scale, Rotate }; // keyboard-only

    // Touch/visual handle ids. SCALE = 8 (corners + edge mids of the
    // selection bbox); ROTATE = 4 corners displaced 28px along the diagonal
    // plus a center crosshair.
    enum class Handle {
        NONE = -1,
        TL, T, TR, R, BR, B, BL, L, // scale/stretch
        ROT_TL, ROT_TR, ROT_BR, ROT_BL, CENTER, // rotate
    };

    // Selection-cue stub (Inkscape draws bbox overlays via this).
    struct SelCue {
        void setBboxesVisible(bool /*visible*/) {}
    };

    SelTrans(SPDesktop *desktop);
    ~SelTrans();

    // ---- API consumed by the select-tool.cpp port (verbatim call sites) ----
    void increaseState(); // scale <-> rotate (toggled by TAP on selected item)
    void resetState(State state = STATE_SCALE);
    void setCenter(Geom::Point const &p);
    void grab(Geom::Point const &p, double /*x*/, double /*y*/, bool /*show_handles*/, bool translating);
    bool moveTo(Geom::Point const &xy, unsigned /*modifiers*/);
    void ungrab();
    void stamp(bool /*clone*/ = false) {}
    void grab_stkey(Geom::Point const &, StickyTransform) {}
    void pause_stkey() {}
    bool request_stkey(Geom::Point const &, unsigned) { return false; }
    bool ungrab_stkey(bool) { return false; }
    bool is_stkey() const { return false; }
    bool isGrabbed() const { return _grabbed; }
    bool isEmpty() const; // no selection -> no transformation
    void getNextClosestPoint(bool /*next*/) {}
    void setShow(Show s) { _show = s; }
    SelCue &getSelCue() { static SelCue cue; return cue; }

    // ---- touch handle engine (renderer-driven) ----
    State state() const { return _state; }
    bool visible() const { return !isEmpty() && _show == SHOW_CONTENT; }
    static double handleRadius(); // screen px; /options/touch/handle_radius
    bool tryGrabHandle(Geom::Point const &p); // DOWN over a visible handle
    bool draggingHandle() const { return _drag_handle != Handle::NONE; }
    Handle currentHandle() const { return _drag_handle; }
    bool moveHandle(Geom::Point const &p); // drag the grabbed handle
    void releaseHandle();                  // UP / cancel

    struct HandlePos { Handle id; Geom::Point pos; };
    // Positions of the currently visible handles (empty when not visible).
    std::vector<HandlePos> handlePositions() const;

    // Union of the selected items' document-space bboxes (empty if none).
    Geom::OptRect bbox() const;
    // Rotation center: explicit (setCenter) or selection bbox center.
    Geom::Point center() const;

private:
    // Snapshot needed to make a drag start clean (no jump to the finger).
    struct GrabState {
        std::vector<std::pair<SPRect *, Geom::Xform>> start_xforms;
        Geom::Rect bbox0;      // selection bbox at grab time
        std::array<Geom::Point, 4> quad0; // transformed box corners at grab
        Geom::Point grab_pos;  // where the drag started (doc space)
        Geom::Point center0;   // rotation center at grab time
        double last_ang = 0.0; // finger angle around center0 (rotate drag)
        double rot_accum = 0.0; // accumulated rotation since grab (continuous)
    };

    void snapshot();
    void initRotateState(); // seed last_ang/rot_accum from grab_pos vs center0
    std::vector<SPRect *> selectedRects() const;
    // The selection's *transformed* outline: a single rect = its four
    // doc-space corners (a rotated quad); multi-selection = union AABB.
    std::array<Geom::Point, 4> selectionQuad() const;

    SPDesktop *_desktop;
    State _state = STATE_SCALE;
    Show _show = SHOW_CONTENT;
    bool _grabbed = false;
    bool _translating = false; // grab was a move-drag (select tool), not handle
    Handle _drag_handle = Handle::NONE;
    bool _center_explicit = false;
    Geom::Point _center;
    GrabState _g;
};

// The live SelTrans (set by the SelectTool ctor, cleared by its dtor); lets
// the renderer drive the handle engine without knowing the tool type.
SelTrans *active_seltrans();

} // namespace Inkscape

#endif // SHAM_SELTRANS_H