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
 *
 * The select tool adds the item transform: SPRect::xform maps the rect's
 * local geometry (x/y/w/h/rx/ry) into document space. The renderer applies
 * it, and hit-testing / selection-cue math invert it — the same semantic as
 * Inkscape's item->transform + i2dt_affine().
 */
#ifndef SHAM_SP_RECT_H
#define SHAM_SP_RECT_H

#include "geom_min.h"
#include "ink_compat.h"
#include <algorithm>
#include <array>
#include <vector>

class SPDocument; // defined in desktop.h

// ---- minimal Inkscape object hierarchy --------------------------------
class SPObject {
public:
    SPDocument *document = nullptr; // alpha: points at the process document
    SPObject *parent = nullptr;     // alpha: flat registry, no parent chain
    Geom::Affine transform;         // rect-tool writes identity here (legacy)
    virtual ~SPObject() = default;
    void requestDisplayUpdate(int /*flags*/) {} // no arena-redraw event in alpha
    void updateRepr() {}                        // no XML repr in alpha
    void doWriteTransform(Geom::Affine const &, SPObject * /*sel*/, bool /*write*/) {}
    bool isAncestorOf(SPObject const *) const { return false; } // flat model
};

namespace Inkscape {
// Arena drawing-item stub: only the cycling (scroll) path touches it, and
// scroll never fires on touch; this keeps the ported code compiling.
class DrawingItem {
public:
    void setOpacityOverride(double) {}
};
} // namespace Inkscape

class SPItem : public SPObject {
public:
    Inkscape::DrawingItem *get_arenaitem(int /*dkey*/) {
        static Inkscape::DrawingItem d;
        return &d;
    }
};
class SPShape : public SPItem {};
class SPGroup : public SPItem {
public:
    enum LayerMode { GROUP, LAYER };
    LayerMode layerMode() const { return _layer_mode; } // alpha: no layers
private:
    LayerMode _layer_mode = GROUP;
};
class SPBox3D : public SPItem {}; // never present in the alpha registry

// is/cast — same helpers Inkscape defines for the object hierarchy.
template <typename T>
bool is(SPObject const *o)
{
    return dynamic_cast<T const *>(o) != nullptr;
}

template <typename T>
T *cast(SPObject *o)
{
    return dynamic_cast<T *>(o);
}

class SPRect; // fwd (SPWeakPtr + registry below)

// Live-rect registry (the alpha's "document"). Declared here so the inline
// SPRect methods can use it; defined after the class. The renderer draws
// every entry every frame; the tool-tracked rect doubles as the preview.
std::vector<SPRect *> &sprect_registry();

// The process document (desktop.cpp); rects created by the tools point at it
// so ported validity checks like `item->document == nullptr` behave like
// Inkscape (never true for live document objects).
SPDocument *the_document();

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

    // Item transform: local geometry (x/y/w/h/rx/ry) -> document space.
    // Initialized to identity (matching a fresh Inkscape item); the select
    // tool composes move/scale/rotate into it while dragging.
    Geom::Xform xform;

    // Alpha: direct creation (no XML node). Port note: the original tool
    // creates an "<svg:rect>" element in the document repr and hydrates an
    // SPRect via appendChildRepr + style + transform; the XML bridge arrives
    // with the real document model phase.
    static SPRect *create() {
        auto *r = new SPRect();
        r->document = the_document();
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

    // ---- geometry helpers (alpha document math) ------------------------
    // Local (untransformed) rectangle: the shape as drawn by rect-tool.
    Geom::Rect localRect() const {
        return Geom::Rect(Geom::Point(x.computed, y.computed),
                          Geom::Point(x.computed + width.computed, y.computed + height.computed));
    }

    // The four document-space corners of the (possibly transformed) rect.
    std::array<Geom::Point, 4> docCorners() const {
        Geom::Rect l = localRect();
        return {xform.apply(l.min()),
                xform.apply({l.max()[Geom::X], l.min()[Geom::Y]}),
                xform.apply(l.max()),
                xform.apply({l.min()[Geom::X], l.max()[Geom::Y]})};
    }

    // Document-space bounding box (AABB of the transformed corners).
    Geom::Rect docBBox() const {
        auto c = docCorners();
        return Geom::Rect(Geom::Point(std::min({c[0].x, c[1].x, c[2].x, c[3].x}),
                                      std::min({c[0].y, c[1].y, c[2].y, c[3].y})),
                          Geom::Point(std::max({c[0].x, c[1].x, c[2].x, c[3].x}),
                                      std::max({c[0].y, c[1].y, c[2].y, c[3].y})));
    }

    bool isEmpty() const { return width.computed == 0 || height.computed == 0; }
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

// refcounting hooks (no-op: the alpha's objects live in the registry, the
// tools never free them on release — same lifetime as Inkscape's document).
inline void sp_object_ref(SPObject *, void * = nullptr) {}
inline void sp_object_unref(SPObject *, void * = nullptr) {}

#endif // SHAM_SP_RECT_H