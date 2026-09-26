// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * layer_manager.h — stub of Inkscape's LayerManager.
 *
 * The alpha has a flat document (no groups/layers yet): currentLayer() is
 * always null, so select-tool's "up one level" / "enter group" paths no-op
 * exactly like Inkscape does when there is no layer under the cursor.
 */
#ifndef SHAM_LAYER_MANAGER_H
#define SHAM_LAYER_MANAGER_H

class SPObject;

namespace Inkscape {

class LayerManager {
public:
    SPObject *currentLayer() const { return nullptr; }
    void setCurrentLayer(SPObject * /*layer*/) {}
};

} // namespace Inkscape

#endif // SHAM_LAYER_MANAGER_H