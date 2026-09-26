// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * desktop.h — stub of SPDesktop + SPNamedView + SPDocument for the alpha.
 *
 * In the real port SPDesktop is the hub (canvas coords <-> doc coords,
 * document, named view, snap manager, message stack, layer manager,
 * hit-testing). The alpha exposes the surface both ported tools touch:
 *   w2d()/d2w()/dt2doc()  : 1:1 identity (no zoom yet)
 *   getSelection()        : process singleton Selection (selection.h)
 *   getItemAtPoint()…     : real hit-testing over the live rect registry
 *   getCanvasDrawing/Catchall/getCanvas : no-op CanvasItem/Canvas stubs
 *   messageStack(), layerManager(), point(), yaxisdir(), dkey…
 *
 * note: in Inkscape SPDesktop is a global-namespace class (the tool headers
 * use it unqualified); we mirror that. SPDocument lives here too and answers
 * getItemsInBox/getItemsPartiallyInBox by scanning the registry (same
 * semantics as document->getItemsInBox scanning the tree).
 */
#ifndef SHAM_DESKTOP_H
#define SHAM_DESKTOP_H

#include "geom_min.h"
#include "snap.h"
#include "canvas_item.h"   // CanvasItem / Canvas
#include "message.h"       // MessageStack
#include "layer_manager.h" // Inkscape::LayerManager
#include "sp_rect.h"       // SPItem + live rect registry

#include <string>
#include <vector>

class SPDesktop;

namespace Inkscape {
class Selection;
class SPNamedView {
public:
    std::string display_units = "px"; // alpha: 1:1 px (no zoom)
    SnapManager snap_manager;         // stub snap manager (snap.h)
};
} // namespace Inkscape

// SnapIndicator: the snap-point feedback cursor the tool updates while
// dragging (remove_snaptarget / set_new_snaptarget). No-op in the alpha.
class SnapIndicator {
public:
    void remove_snaptarget() {}
    void set_new_snaptarget(Inkscape::SnappedPoint const &) {}
};

// Alpha document placeholder — becomes the real SPDocument later. The
// registry-scan helpers answer the select tool's rubberband selection.
class SPDocument {
public:
    ~SPDocument() = default;
    void ensureUpToDate() {} // no lazy text layout in the alpha

    // RECT mode: items fully inside the box (all 4 transformed corners).
    std::vector<SPItem *> getItemsInBox(int /*dkey*/, Geom::Rect const &box) const;

    // TOUCHRECT mode: items whose transformed bbox intersects the box.
    std::vector<SPItem *> getItemsPartiallyInBox(int /*dkey*/, Geom::Rect const &box) const;

    // TOUCHPATH mode: items under the given doc-space points (topmost per
    // point, deduped — Inkscape's semantics for touch selection).
    std::vector<SPItem *> getItemsAtPoints(std::vector<Geom::Point> const &points,
                                           bool topmost_only, bool /*ignore_mouseover*/) const;
};

// SPDesktop (global namespace like Inkscape; stays global per convention).
class SPDesktop {
public:
    SPDesktop();
    ~SPDesktop();

    Geom::Point w2d(Geom::Point const &p) const { return p; } // 1:1 alpha
    Geom::Point d2w(Geom::Point const &p) const { return p; }
    Geom::Affine dt2doc() const { return Geom::Affine(); }

    Inkscape::Selection *getSelection() const;
    Inkscape::SPNamedView *getNamedView() const { return namedview; }
    SnapIndicator *getSnapIndicator() const { return snapindicator; }
    SPDocument *getDocument() const { return document; }
    SPDocument *doc() const { return document; }
    Inkscape::MessageStack *messageStack() const { return message_stack; }
    Inkscape::LayerManager &layerManager() const { return *layer_manager; }

    // Hit-testing over the live registry (real, topmost-first). upto skips
    // that item (used by the alt-scroll cycle through stacked items).
    SPItem *getItemAtPoint(Geom::Point const &p, bool into_groups, SPItem *upto = nullptr) const;
    SPItem *getGroupAtPoint(Geom::Point const & /*p*/) const { return nullptr; } // flat model
    SPItem *getItemFromListAtPointBottom(std::vector<SPItem *> const &list,
                                         Geom::Point const &p) const;
    std::vector<SPItem *> getItemsAtPoints(std::vector<Geom::Point> const &points,
                                           bool topmost_only,
                                           bool ignore_mouseover = false) const;

    // Canvas stubs: the alpha's single event dispatcher routes to the tool
    // directly (see Renderer::touch); the grab targets exist for the port.
    CanvasItem *getCanvasDrawing() const { return drawing; }
    CanvasItem *getCanvasCatchall() const { return catchall; }
    Canvas *getCanvas() const { return canvas; }

    // Desktop state the ported handlers read.
    Geom::Point point() const { return desk_point; } // last known pointer pos
    void setPoint(Geom::Point const &p) { desk_point = p; }
    double yaxisdir() const { return 1.0; } // y grows downwards (SVG)
    bool isWaitingCursor() const { return false; }
    int dkey = 0; // view key (used for arena item lookups; 0 default)
    int tolerance = 0; // (kept for symmetry with ToolBase)

private:
    Inkscape::SPNamedView *namedview = nullptr;
    SnapIndicator *snapindicator = nullptr;
    SPDocument *document = nullptr;
    Inkscape::MessageStack *message_stack = nullptr;
    Inkscape::LayerManager *layer_manager = nullptr;
    CanvasItem *drawing = nullptr;
    CanvasItem *catchall = nullptr;
    Canvas *canvas = nullptr;
    Geom::Point desk_point;
};

// Process document singleton (SPRect::create() points every new rect at it,
// mirroring Inkscape: items created in a document carry that document).
SPDocument *the_document();

#endif // SHAM_DESKTOP_H