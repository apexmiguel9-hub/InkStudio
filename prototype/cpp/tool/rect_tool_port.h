// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * rect_tool_port.h — port of Inkscape ui/tools/rect-tool.h (alpha).
 *
 * Header preserved 1:1 from the original; only the includes are redirected
 * to the sham/ harness, and the sigc connection is a trivial stub.
 */
#ifndef INKALPHA_TOOLS_RECT_TOOL_H
#define INKALPHA_TOOLS_RECT_TOOL_H

#include "tool_base.h" // original: ui/tools/tool-base.h
#include "sp_rect.h"   // original: object/weakptr.h (provides SPWeakPtr)

class SPRect;

namespace Inkscape { class Selection; }

namespace Inkscape::UI::Tools {

class RectTool : public ToolBase
{
public:
    RectTool(SPDesktop *desktop);
    ~RectTool() override;

    void set(Preferences::Entry const &val) override;
    bool root_handler(CanvasEvent const &event) override;
    bool item_handler(SPItem *item, CanvasEvent const &event) override;

private:
    SPWeakPtr<SPRect> rect;
    Geom::Point center;

    double rx; /* roundness radius (x direction) */
    double ry; /* roundness radius (y direction) */

    // original: sigc::connection sel_changed_connection;
    // ADAPTACIÓN (alpha): sin sigc/signals todavía; la UI de selección se
    // conecta en la fase de documento+selector.
    struct Connection { void disconnect() {} } sel_changed_connection;

    Modifiers::Modifier *mod_select_add_to;
    Modifiers::Modifier *mod_select_force_drag;
    Modifiers::Modifier *mod_trans_confine;
    Modifiers::Modifier *mod_trans_off_center;

    void drag(Geom::Point const pt, unsigned state);
    void finishItem();
    void cancel();
    void selection_changed(Selection *selection);
};

} // namespace Inkscape::UI::Tools

#endif // INKALPHA_TOOLS_RECT_TOOL_H