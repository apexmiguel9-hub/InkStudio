// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * sp_rect.h — SPRect object-model subset (alpha).
 *
 * The rect tool's port drives an SPRect (created on first drag motion,
 * updated per motion via setPosition/setRx/setRy, confirmed in finishItem).
 * The formulas below are copied verbatim from Inkscape's object/sp-rect.cpp;
 * only the Inkscape-object-system plumbing is stripped (no SPDocument/XML/
 * style), and the "live rect registry" stands in for the document (the
 * renderer draws every entry — the same semantic as the arena painting the
 * tree). requestDisplayUpdate() is a no-op (in Inkscape it schedules a
 * Cairo/arena redraw — in InkAlpha the renderer re-reads geometry per frame).
 */
#ifndef SHAM_SP_RECT_H
#define SHAM_SP_RECT_H

#include "geom_min.h"
#include "ink_compat.h"
#include <algorithm>
#include <vector>

// ---- minimal Inkscape object hierarchy --------------------------------
class SPObject {
public:
    Geom::Affine transform; // item transform (alpha: identity)
    virtual ~SPObject() = default;
    void requestDisplayUpdate(int /*flags*/) {} // no arena-redraw event in alpha
    void updateRepr() {}                        // no XML repr in alpha
    void doWriteTransform(Geom::Affine const &, SPObject * /*sel*/, bool /*write*/) {}
};

class SPItem : public SPObject {};
class SPShape : public SPItem {};
class SPGroup : public SPItem {};

class SPRect; // fwd (SPWeakPtr + registry below)

// Live-rect registry (the alpha's "document"). Declared here so the inline
// SPRect methods can use it; defined after the class. The renderer draws
// every entry every frame; the tool-tracked rect doubles as the preview.
std::vector<SPRect *> &sprect_registry();

// ---- SPRect ------------------------------------------------------------
class SPRect : public SPShape {
public:
    // SVGLength subset: value + computed + _set (as in object/svg-length.h)
    struct SVGLength {
        double value = 0.0;
        double computed = 0.0;
        bool _set = false;
        SVGLength &operator=(double v) { value = v; computed = v; return *this; }
    };

    SVGLength x, y, width, height, rx, ry;

    // Alpha: direct creation (no XML node). Port note: the original tool
    // creates an "<svg:rect>" element in the document repr and hydrates an
    // SPRect via appendChildRepr + style + transform; the XML bridge arrives
    // with the real document model phase.
    static SPRect *create() {
        auto *r = new SPRect();
        sprect_registry().push_back(r);
        return r;
    }

    void deleteObject() {
        auto &reg = sprect_registry();
        reg.erase(std::remove(reg.begin(), reg.end(), this), reg.end());
        delete this;
    }

    // Verbatim from object/sp-rect.cpp
    void setPosition(gdouble px, gdouble py, gdouble pw, gdouble ph) {
        this->x = px;
        this->y = py;
        this->width = pw;
        this->height = ph;
        this->requestDisplayUpdate(1 /* SP_OBJECT_MODIFIED_FLAG */);
    }

    // Verbatim from object/sp-rect.cpp
    void setRx(bool set, gdouble value) {
        this->rx._set = set;
        if (set) {
            this->rx = value;
        }
        this->requestDisplayUpdate(1 /* SP_OBJECT_MODIFIED_FLAG */);
    }

    // Verbatim from object/sp-rect.cpp
    void setRy(bool set, gdouble value) {
        this->ry._set = set;
        if (set) {
            this->ry = value;
        }
        this->requestDisplayUpdate(1 /* SP_OBJECT_MODIFIED_FLAG */);
    }
};

// Weak-pointer surface used by the tool (SPWeakPtr<SPRect> rect).
template <typename T>
class SPWeakPtr {
    T *p = nullptr;
public:
    SPWeakPtr() = default;
    SPWeakPtr(T *ptr) : p(ptr) {}
    SPWeakPtr<T> &operator=(T *ptr) { p = ptr; return *this; }
    T *get() const { return p; }
    T *operator->() const { return p; }
    explicit operator bool() const { return p != nullptr; }
};

// Live-rect registry = the alpha's "document". The renderer draws every
// entry every frame; the tool-tracked rect doubles as the drag preview.
inline std::vector<SPRect *> &sprect_registry() {
    static std::vector<SPRect *> reg;
    return reg;
}

// path-effect hook referenced by finishItem() (no LPEs in the alpha)
inline void sp_lpe_item_update_patheffect(SPObject *, bool, bool) {}

#endif // SHAM_SP_RECT_H