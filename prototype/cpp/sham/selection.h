// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * selection.h — Inkscape::Selection subset for the alpha.
 *
 * A real, ordered selection over the live object registry (the alpha's
 * "document"). surface mirrors Inkscape's Selection.h for the two ported
 * tools:
 *   - rect-tool:  set(), singleItem(), toggle(), includes(), clear(),
 *   - select-tool: set/setList/addList/removeList/add/remove/toggle/clear,
 *     isEmpty(), includes(item, into_groups), single(), items(),
 *     duplicate(), move(), toGuides().
 *
 * Insertion order is z-order (front = topmost), like Inkscape's selection.
 */
#ifndef SHAM_SELECTION_H
#define SHAM_SELECTION_H

#include "sp_rect.h" // SPObject / SPItem + registry
#include <vector>

namespace Inkscape {

class Selection {
public:
    void set(SPObject *sel) {
        _items.clear();
        if (sel) {
            _items.push_back(static_cast<SPItem *>(sel));
        }
    }

    void setList(std::vector<SPItem *> const &items) {
        _items = items;
    }

    void add(SPObject *o) {
        if (o) {
            _items.push_back(static_cast<SPItem *>(o));
        }
    }

    void addList(std::vector<SPItem *> const &items) {
        for (auto *item : items) {
            if (item) {
                _items.push_back(item);
            }
        }
    }

    void remove(SPObject *o) {
        auto it = std::find(_items.begin(), _items.end(), static_cast<SPItem *>(o));
        if (it != _items.end()) {
            _items.erase(it);
        }
    }

    void removeList(std::vector<SPItem *> const &items) {
        for (auto *item : items) {
            remove(item);
        }
    }

    void toggle(SPObject *o) {
        if (includes(o)) {
            remove(o);
        } else {
            add(o);
        }
    }

    void clear() { _items.clear(); }

    bool isEmpty() const { return _items.empty(); }

    // into_groups: the alpha has no groups; the flag is accepted for the
    // ported call sites and has no extra semantics.
    bool includes(SPObject const *o, bool /*into_groups*/ = false) const {
        return std::find(_items.begin(), _items.end(), static_cast<SPItem const *>(o)) != _items.end();
    }

    // Topmost selected item (front of the list) in document z-order.
    std::vector<SPItem *> items() const { return _items; }

    SPObject *single() const { return _items.empty() ? nullptr : _items.front(); }
    SPItem *singleItem() const { return _items.empty() ? nullptr : _items.front(); }

    // Duplicate-drag hook: real Inkscape clones the selection; the alpha has
    // no clone semantics yet (the drag just moves the original). Declared so
    // the port compiles; returns null like "nothing new was created".
    SPItem *duplicate(bool /*rel*/ = false) { return nullptr; }

    // Keyboard nudge (arrow keys). Translates every selected rect in doc
    // space (screen-space == doc-space in the 1:1 alpha).
    void move(double dx, double dy, bool /*rotated*/, bool /*screen*/) {
        for (auto *item : _items) {
            if (auto *r = dynamic_cast<SPRect *>(item)) {
                r->xform = Geom::Xform::translate(dx, dy) * r->xform;
            }
        }
    }

    void toGuides() {} // Shift+G: convert-to-guides (no guides in the alpha)

    // signal "connection" API used by the constructor in the original code;
    // the alpha does not connect anything (see adaptation note in the port).
    struct Connection { void disconnect() {} };

private:
    std::vector<SPItem *> _items; // ordered, front = topmost
};

} // namespace Inkscape

#endif // SHAM_SELECTION_H