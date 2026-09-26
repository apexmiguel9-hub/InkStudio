// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * rubberband.h — Inkscape::Rubberband subset for the alpha.
 *
 * Real drag-state (start/move/stop + rectangle + points + mode/operation),
 * rendered by the renderer as a translucent overlay. The rubberband itself is
 * the alpha's equivalent of Inkscape's canvas-item ctrl; selection semantics
 * (getItemsInBox etc.) are answered by the desktop/document stubs.
 *
 * default_mode = RECT (confirmed: the alpha stays on RECT; TOUCHRECT/TOUCHPATH
 * only activate via modifiers which never fire on touch).
 */
#ifndef SHAM_RUBBERBAND_H
#define SHAM_RUBBERBAND_H

#include "geom_min.h"
#include <algorithm>
#include <vector>

class SPDesktop;

namespace Inkscape {

// Canvas ctrl types the tool hands the rubberband (kept for port fidelity;
// the alpha draws them all the same way).
enum class CanvasItemCtrlType {
    RUBBERBAND_RECT,
    RUBBERBAND_TOUCHRECT,
    RUBBERBAND_TOUCHPATH_SELECT,
};

class Rubberband {
public:
    enum class Mode { RECT, TOUCHRECT, TOUCHPATH };
    enum class Operation { ADD, REMOVE, INVERT };

    // The tool queries the *default* state via get_default_rubberband_state()
    // and stores mode/handle on the instance; defaults stay RECT (confirmed:
    // /tools/select/touch_box pref remains false in the alpha).
    inline static Mode default_mode = Mode::RECT;
    inline static CanvasItemCtrlType default_handle = CanvasItemCtrlType::RUBBERBAND_RECT;

    static Rubberband *get(SPDesktop * /*desktop*/) {
        static Rubberband rb;
        return &rb;
    }

    void start(SPDesktop * /*desktop*/, Geom::Point const &p, bool /*is_node_tool*/ = false) {
        _start = p;
        _current = p;
        _started = true;
        _moved = false;
        _points.clear();
        _points.push_back(p);
    }

    void move(Geom::Point const &p) {
        _current = p;
        _moved = true;
        _points.push_back(p);
    }

    void stop() {
        _started = false;
        _moved = false;
        _points.clear();
    }

    bool isStarted() const { return _started; }
    bool isMoved() const { return _moved; }

    Geom::OptRect getRectangle() const {
        if (!_started) {
            return std::nullopt;
        }
        return Geom::Rect(Geom::Point(std::min(_start.x, _current.x),
                                      std::min(_start.y, _current.y)),
                          Geom::Point(std::max(_start.x, _current.x),
                                      std::max(_start.y, _current.y)));
    }

    std::vector<Geom::Point> const &getPoints() const { return _points; }

    Mode getMode() const { return _mode; }
    void setMode(Mode m) { _mode = m; }

    Operation getOperation() const { return _operation; }
    void setOperation(Operation o) { _operation = o; }

    CanvasItemCtrlType getHandle() const { return _handle; }
    void setHandle(CanvasItemCtrlType h) { _handle = h; }

private:
    bool _started = false;
    bool _moved = false;
    Geom::Point _start;
    Geom::Point _current;
    Mode _mode = Mode::RECT;
    Operation _operation = Operation::ADD;
    CanvasItemCtrlType _handle = CanvasItemCtrlType::RUBBERBAND_RECT;
    std::vector<Geom::Point> _points;
};

} // namespace Inkscape

#endif // SHAM_RUBBERBAND_H