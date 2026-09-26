// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * desktop.h — stub of SPDesktop + SPNamedView for the alpha.
 *
 * In the real port SPDesktop is the hub (canvas coords <-> doc coords,
 * document, named view, snap manager, message context...). Here we expose the
 * surface rect-tool.cpp touches:
 *   w2d()        : canvas -> doc transform (identity: 1:1, no zoom yet)
 *   dt2doc()     : desktop-space -> doc-space affine (identity)
 *   getSelection(), getNamedView(), getSnapIndicator(), getDocument()
 *
 * Note: in Inkscape SPDesktop is a global-namespace class (the tool headers
 * use it unqualified); we mirror that.
 */
#ifndef SHAM_DESKTOP_H
#define SHAM_DESKTOP_H

#include "geom_min.h"
#include "snap.h"
#include <string>

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

// Alpha document placeholder — becomes the real SPDocument later.
class SPDocument {
public:
    ~SPDocument() = default;
};

// SPDesktop (global namespace like Inkscape; stays global per convention).
class SPDesktop {
public:
    SPDesktop();
    ~SPDesktop();

    Geom::Point w2d(Geom::Point const &p) const { return p; } // 1:1 alpha
    Geom::Affine dt2doc() const { return Geom::Affine(); }

    Inkscape::Selection *getSelection() const;
    Inkscape::SPNamedView *getNamedView() const { return namedview; }
    SnapIndicator *getSnapIndicator() const { return snapindicator; }
    SPDocument *getDocument() const { return document; }
    SPDocument *doc() const { return document; }

private:
    Inkscape::SPNamedView *namedview = nullptr;
    SnapIndicator *snapindicator = nullptr;
    SPDocument *document = nullptr;
};

#endif // SHAM_DESKTOP_H