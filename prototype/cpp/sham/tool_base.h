// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * tool_base.h — ToolBase subset used by the rect tool port.
 *
 * Mirror of Inkscape's ui/tools/tool-base.h surface (fields + the drag
 * helpers the tool interacts with). The implementations of the three drag
 * helpers (saveDragOrigin/checkDragMoved/setup_for_drag_start) are verbatim
 * from ui/tools/tool-base.cpp; everything else is a no-op stub for the alpha.
 */
#ifndef SHAM_TOOL_BASE_H
#define SHAM_TOOL_BASE_H

#include "canvas_events.h"
#include "desktop.h"
#include "message.h"
#include "mods.h"
#include "prefs.h"
#include "sp_rect.h"
#include <memory>
#include <string>

namespace Inkscape::UI {

// ui/shape-editor.h stub (selection knotholders) — no-op in the alpha.
class ShapeEditor {
public:
    ShapeEditor(SPDesktop *) {}
    void set_item(SPObject *) {}
    void unset_item() {}
};

namespace Tools {

class ToolBase {
public:
    ToolBase(SPDesktop *desktop, std::string &&prefs_path, std::string &&cursor_filename,
             bool /*uses_snap*/ = true)
        : _desktop(desktop)
        , _prefs_path(std::move(prefs_path))
        , message_context(std::make_unique<Inkscape::MessageContext>()) {}

    virtual ~ToolBase() = default;

    virtual void set(Preferences::Entry const &) {}
    virtual bool root_handler(CanvasEvent const &) { return false; }
    virtual bool item_handler(SPItem *, CanvasEvent const &) { return false; }

    void enableSelectionCue(bool = true) {}
    void enableGrDrag(bool = true) {}

    MessageContext *defaultMessageContext() const { return message_context.get(); }
    SPDesktop *getDesktop() const { return _desktop; }
    SPGroup *currentLayer() const { return nullptr; } // alpha: no layer model

protected:
    SPDesktop *_desktop;
    std::string _prefs_path;
    std::unique_ptr<Inkscape::MessageContext> message_context;

    // verbatim member set from tool-base.h
    Geom::IntPoint xyp;            ///< where drag started
    bool dragging = false;         ///< are we dragging?
    int tolerance = 0;
    bool within_tolerance = false; ///< are we still within tolerance of origin
    bool _button1on = false;
    bool _button2on = false;
    bool _button3on = false;
    SPItem *item_to_select = nullptr; ///< item under the press, for click-select

    Inkscape::UI::ShapeEditor *shape_editor = nullptr;

    // drag helpers (implementations verbatim from tool-base.cpp)
    void setup_for_drag_start(ButtonPressEvent const &event);
    void saveDragOrigin(Geom::Point const &pos);
    bool checkDragMoved(Geom::Point const &pos);

    // no-op event-grab/misc hooks (alpha has a single event consumer)
    void grabCanvasEvents(unsigned /*mask*/ = 0) {}
    void ungrabCanvasEvents() {}
    void gobble_motion_events(unsigned) {}
    void discard_delayed_snap_event() {}
    bool deleteSelectedDrag(bool) { return false; } // no selection deletion yet
};

} // namespace Tools
} // namespace Inkscape::UI

#endif // SHAM_TOOL_BASE_H