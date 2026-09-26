// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * desktop.cpp — construction of the alpha's SPDesktop stub + hit-testing.
 *
 * The pointers are stubs; the interesting part is getItemAtPoint and the
 * document box-queries, which are REAL: they scan the live rect registry in
 * z-order (back to front) and invert each item's transform to answer
 * "is this document point inside this (possibly transformed) rect?".
 */
#include "desktop.h"
#include "selection.h"

#include <algorithm>
#include <cmath>

SPDesktop::SPDesktop()
{
    namedview = new Inkscape::SPNamedView();
    snapindicator = new SnapIndicator();
    document = the_document(); // one document per process
    message_stack = new Inkscape::MessageStack();
    layer_manager = new Inkscape::LayerManager();
    drawing = new CanvasItem();
    catchall = new CanvasItem();
    canvas = new Canvas();
}

SPDesktop::~SPDesktop()
{
    delete namedview;
    delete snapindicator;
    // document is the process singleton (the_document()): not owned here.
    delete message_stack;
    delete layer_manager;
    delete drawing;
    delete catchall;
    delete canvas;
}

SPDocument *the_document()
{
    static SPDocument doc;
    return &doc;
}

Inkscape::Selection *SPDesktop::getSelection() const
{
    // Alpha: a single selection singleton per process (no multi-desktop yet).
    static Inkscape::Selection sel;
    return &sel;
}

// ---- hit-testing over the live registry --------------------------------

// Inverse-transform p into the item's local space and check the local rect.
static bool item_contains(SPItem *item, Geom::Point const &p_doc)
{
    auto *r = dynamic_cast<SPRect *>(item);
    if (!r || r->isEmpty()) {
        return false;
    }
    Geom::Point local = r->xform.inverse().apply(p_doc);
    return r->localRect().contains(local);
}

SPItem *SPDesktop::getItemAtPoint(Geom::Point const &p, bool /*into_groups*/, SPItem *upto) const
{
    // Registry is in creation order (back first, front last): scan backwards
    // so the topmost (front) item wins, exactly like Inkscape's arena hit.
    auto &reg = sprect_registry();
    for (auto it = reg.rbegin(); it != reg.rend(); ++it) {
        SPItem *item = *it;
        if (item == upto) {
            continue;
        }
        if (item_contains(item, p)) {
            return item;
        }
    }
    return nullptr;
}

SPItem *SPDesktop::getItemFromListAtPointBottom(std::vector<SPItem *> const &list,
                                                Geom::Point const &p) const
{
    // The list is topmost-first; the bottom-most item at p is the last
    // matching entry (used by select_under / alt+click).
    SPItem *bottom = nullptr;
    for (SPItem *item : list) {
        if (item_contains(item, p)) {
            bottom = item;
        }
    }
    return bottom;
}

std::vector<SPItem *> SPDesktop::getItemsAtPoints(std::vector<Geom::Point> const &points,
                                                  bool topmost_only, bool /*ignore_mouseover*/) const
{
    std::vector<SPItem *> items;
    if (topmost_only) {
        for (auto const &p : points) {
            if (SPItem *item = getItemAtPoint(p, true); item) {
                if (std::find(items.begin(), items.end(), item) == items.end()) {
                    items.push_back(item);
                }
            }
        }
    } else {
        // all items under each point, topmost first
        auto &reg = sprect_registry();
        for (auto const &p : points) {
            for (auto it = reg.rbegin(); it != reg.rend(); ++it) {
                SPItem *item = *it;
                if (item_contains(item, p) &&
                    std::find(items.begin(), items.end(), item) == items.end()) {
                    items.push_back(item);
                }
            }
        }
    }
    return items;
}

// ---- document box queries ---------------------------------------------

static bool box_overlaps_aabb(Geom::Rect const &box, Geom::Rect const &other)
{
    return !(box.min().x >= other.max().x || other.min().x >= box.max().x ||
             box.min().y >= other.max().y || other.min().y >= box.max().y);
}

std::vector<SPItem *> SPDocument::getItemsInBox(int /*dkey*/, Geom::Rect const &box) const
{
    std::vector<SPItem *> items;
    for (auto *r : sprect_registry()) {
        if (r->isEmpty()) {
            continue;
        }
        // fully inside: all 4 transformed corners inside the rubberband box
        auto corners = r->docCorners();
        bool inside = true;
        for (auto const &c : corners) {
            if (!box.contains(c)) {
                inside = false;
                break;
            }
        }
        if (inside) {
            items.push_back(r);
        }
    }
    return items;
}

std::vector<SPItem *> SPDocument::getItemsPartiallyInBox(int /*dkey*/, Geom::Rect const &box) const
{
    std::vector<SPItem *> items;
    for (auto *r : sprect_registry()) {
        if (r->isEmpty()) {
            continue;
        }
        if (box_overlaps_aabb(box, r->docBBox())) {
            items.push_back(r);
        }
    }
    return items;
}

std::vector<SPItem *> SPDocument::getItemsAtPoints(std::vector<Geom::Point> const &points,
                                                   bool topmost_only,
                                                   bool ignore_mouseover) const
{
    std::vector<SPItem *> items;
    for (auto const &p : points) {
        if (topmost_only) {
            auto &reg = sprect_registry();
            for (auto it = reg.rbegin(); it != reg.rend(); ++it) {
                SPItem *item = *it;
                if (item_contains(item, p)) {
                    if (std::find(items.begin(), items.end(), item) == items.end()) {
                        items.push_back(item);
                    }
                    break;
                }
            }
        } else {
            auto &reg = sprect_registry();
            for (auto it = reg.rbegin(); it != reg.rend(); ++it) {
                SPItem *item = *it;
                if (item_contains(item, p) &&
                    std::find(items.begin(), items.end(), item) == items.end()) {
                    items.push_back(item);
                }
            }
        }
    }
    return items;
}