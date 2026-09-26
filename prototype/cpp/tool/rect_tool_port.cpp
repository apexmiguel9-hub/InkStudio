// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * rect_tool_port.cpp — port of Inkscape's ui/tools/rect-tool.cpp (alpha).
 *
 * The interaction algorithm below is copied VERBATIM from the Inkscape
 * sources (the whole ButtonPress/Motion/ButtonRelease/Key logic, drag(),
 * finishItem(), cancel(), item_handler/root_handler dispatch). Only two
 * kinds of changes were made:
 *
 *   1. INCLUDES: redirected from Inkse-scape/GTK headers to the sham/
 *      harness (which mirrors the exact service surface the tool uses:
 *      ToolBase drag helpers, CanvasEvent hierarchy, Preferences,
 *      MessageContext, Selection, DocumentUndo, snap_rectangular_box...).
 *   2. The XML-node creation block in drag() (adapted; see marker), and the
 *      selection signal connection in the constructor (sigc).
 *
 * Everything else — how the drag is detected, how the box is computed from
 * origin+current point (snap_rectangular_box), how the rect object is
 * updated while dragging, how it is confirmed or cancelled on release — is
 * unchanged Inkscape code.
 */
#include "ink_compat.h"    // _(), Glib::ustring, GDK constants, CLAMP, RC_
#include "canvas_events.h" // CanvasEvent + inspect_event + get_latin_keyval + mod_*_only
#include "message.h"       // original <glibmm/i18n.h> + message-context.h
#include "prefs.h"         // Inkscape::Preferences
#include "selection.h"     // Inkscape::Selection
#include "sp_rect.h"       // original: object/sp-rect.h + object/weakptr.h
#include "snap.h"          // original: context-fns.h (snap_rectangular_box + sp_event_context_*)
#include "tool_base.h"     // original: ui/tools/tool-base.h + ui/shape-editor.h
#include "undo.h"          // original: document-undo.h
#include "util.h"          // original: util/units.h (Util::Quantity)
#include "rect_tool_port.h"

using Inkscape::DocumentUndo;

namespace Inkscape::UI::Tools {

RectTool::RectTool(SPDesktop *desktop)
    : ToolBase(desktop, "/tools/shapes/rect", "rect.svg")
    , rx(0)
    , ry(0)
    , mod_select_add_to(Modifiers::Modifier::get(Modifiers::Type::SELECT_ADD_TO))
    , mod_select_force_drag(Modifiers::Modifier::get(Modifiers::Type::SELECT_FORCE_DRAG))
    , mod_trans_confine(Modifiers::Modifier::get(Modifiers::Type::TRANS_CONFINE))
    , mod_trans_off_center(Modifiers::Modifier::get(Modifiers::Type::TRANS_OFF_CENTER))
{
    this->shape_editor = new ShapeEditor(desktop);

    SPItem *item = desktop->getSelection()->singleItem();
    if (item) {
        this->shape_editor->set_item(item);
    }

    // ADAPTACIÓN (alpha): en Inkscape aquí se conecta el signal de cambio de
    // selección (sigc::mem_fun -> selection_changed) para mantener los
    // knotholders del ShapeEditor sincronizados con la selección:
    //
    //   this->sel_changed_connection = desktop->getSelection()->connectChanged(
    //       sigc::mem_fun(*this, &RectTool::selection_changed));
    //
    // El alpha no tiene signals/pickers todavía; la conexión real llega con
    // la fase de documento+selector.

    sp_event_context_read(this, "rx");
    sp_event_context_read(this, "ry");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (prefs->getBool("/tools/shapes/selcue")) {
        this->enableSelectionCue();
    }

    if (prefs->getBool("/tools/shapes/gradientdrag")) {
        this->enableGrDrag();
    }
}

RectTool::~RectTool() {
    ungrabCanvasEvents();

    this->finishItem();
    this->enableGrDrag(false);

    this->sel_changed_connection.disconnect();

    delete this->shape_editor;
    this->shape_editor = nullptr;
}

/**
 * Callback that processes the "changed" signal on the selection;
 * destroys old and creates new knotholder.
 */
void RectTool::selection_changed(Inkscape::Selection* selection) {
    this->shape_editor->unset_item();
    this->shape_editor->set_item(selection->singleItem());
}

void RectTool::set(const Inkscape::Preferences::Entry& val) {
    /* fixme: Proper error handling for non-numeric data.  Use a locale-independent function like
     * g_ascii_strtod (or a thin wrapper that does the right thing for invalid values inf/nan). */
    Glib::ustring name = val.getEntryName();
    
    if ( name == "rx" ) {
        this->rx = val.getDoubleLimited(); // prevents NaN and +/-Inf from messing up
    } else if ( name == "ry" ) {
        this->ry = val.getDoubleLimited();
    }
}

bool RectTool::item_handler(SPItem *item, CanvasEvent const &event)
{
    inspect_event(event,
        [&] (ButtonPressEvent const &event) {
            if (event.num_press == 1 && event.button == 1) {
                setup_for_drag_start(event);
            }
        },
        [&] (CanvasEvent const &event) {}
    );

    return ToolBase::item_handler(item, event);
}

bool RectTool::root_handler(CanvasEvent const &event)
{
    auto selection = _desktop->getSelection();
    auto prefs = Inkscape::Preferences::get();

    tolerance = prefs->getIntLimited("/options/dragtolerance/value", 0, 0, 100);

    bool ret = false;

    inspect_event(event,
        [&] (ButtonPressEvent const &event) {
            if (event.num_press == 1 && event.button == 1) {

                auto const button_w = event.pos;

                // Save drag origin
                saveDragOrigin(button_w);
                dragging = true;

                // Remember clicked item, disregarding groups, honoring Alt.
                bool force_drag = mod_select_force_drag->active(event.modifiers);
                item_to_select = sp_event_context_find_item (_desktop, button_w, force_drag, true);
                // Postion center
                auto button_dt = _desktop->w2d(button_w);
                center = button_dt;

                // Snap center
                auto &m = _desktop->getNamedView()->snap_manager;
                m.setup(_desktop);
                m.freeSnapReturnByRef(button_dt, SNAPSOURCE_NODE_HANDLE);
                m.unSetup();
                center = button_dt;

                grabCanvasEvents();
                ret = true;
            }
        },
        [&] (MotionEvent const &event) {
            if (dragging && (event.modifiers & GDK_BUTTON1_MASK)) {
                if (!checkDragMoved(event.pos)) {
                    return;
                }

                auto const motion_dt = _desktop->w2d(event.pos);
                drag(motion_dt, event.modifiers); // This will also handle the snapping.

                gobble_motion_events(GDK_BUTTON1_MASK);

                ret = true;
            } else if (!sp_event_context_knot_mouseover()) {
                auto &m = _desktop->getNamedView()->snap_manager;
                m.setup(_desktop);

                auto const motion_dt = _desktop->w2d(event.pos);
                m.preSnap(SnapCandidatePoint(motion_dt, SNAPSOURCE_NODE_HANDLE));
                m.unSetup();
            }
        },
        [&] (ButtonReleaseEvent const &event) {
            xyp = {};
            if (dragging && event.button == 1) {
                dragging = false;
                discard_delayed_snap_event();

                if (rect) {
                    // We've been dragging, finish the rect.
                    finishItem();
                } else if (item_to_select) {
                    // No dragging, select clicked item if any.
                    if (mod_select_add_to->active(event.modifiers)) {
                        selection->toggle(item_to_select);
                    } else if (!selection->includes(item_to_select)) {
                        selection->set(item_to_select);
                    }
                } else {
                    // Click in an empty space.
                    selection->clear();
                }

                item_to_select = nullptr;
                ret = true;
            }
            ungrabCanvasEvents();
        },
        [&] (KeyPressEvent const &event) {
            auto keyval = get_latin_keyval(event);

            if (Modifiers::keyval_is_a_modifier(keyval)) {
                if (!dragging) {
                    Modifiers::responsive_tooltip_with_labels(
                        defaultMessageContext(), event, 3,
                        Modifiers::Type::TRANS_CONFINE,
                        _("Make square or integer-ratio rect, lock a rounded corner circular"),
                        Modifiers::Type::TRANS_INCREMENT, _("Use with Ctrl to make square"),
                        Modifiers::Type::TRANS_OFF_CENTER, _("Draw around the starting point")
                    );
                }
            }

            switch (keyval) {
                case GDK_KEY_g:
                case GDK_KEY_G:
                    if (mod_shift_only(event)) {
                        _desktop->getSelection()->toGuides();
                        ret = true;
                    }
                    break;

                case GDK_KEY_Escape:
                    if (dragging) {
                        dragging = false;
                        discard_delayed_snap_event();
                        // if drawing, cancel, otherwise pass it up for deselecting
                        cancel();
                        ret = true;
                    }
                    break;

                case GDK_KEY_space:
                    if (dragging) {
                        ungrabCanvasEvents();
                        dragging = false;
                        this->discard_delayed_snap_event();

                        if (!within_tolerance) {
                            // we've been dragging, finish the rect
                            finishItem();
                        }
                        // do not return true, so that space would work switching to selector
                    }
                    break;

                case GDK_KEY_Delete:
                case GDK_KEY_KP_Delete:
                case GDK_KEY_BackSpace:
                    ret = deleteSelectedDrag(mod_ctrl_only(event));
                    break;

                default:
                    break;
            }
        },
        [&] (KeyReleaseEvent const &event) {
            auto keyval = get_latin_keyval(event);

            if (Modifiers::keyval_is_a_modifier(keyval)) {
                defaultMessageContext()->clear();
            }
        },
        [&] (CanvasEvent const &event) {}
    );

    return ret || ToolBase::root_handler(event);
}

void RectTool::drag(Geom::Point const pt, unsigned state) {
    if (!this->rect) {
        if (Inkscape::have_viable_layer(_desktop, defaultMessageContext()) == false) {
            return;
        }

        // ADAPTACIÓN (alpha): el bloque original crea el shape vía nodo XML
        // del documento:
        //
        //   Inkscape::XML::Document *xml_doc = _desktop->doc()->getReprDoc();
        //   Inkscape::XML::Node *repr = xml_doc->createElement("svg:rect");
        //   _desktop->applyCurrentOrToolStyle(repr, "/tools/shapes/rect", false);
        //   this->rect = cast<SPRect>(currentLayer()->appendChildRepr(repr));
        //   Inkscape::GC::release(repr);
        //   this->rect->transform = currentLayer()->i2doc_affine().inverse();
        //   this->rect->updateRepr();
        //
        // Sin documento XML en el alpha, creamos el SPRect directamente: misma
        // semántica (objeto vivo que se actualiza en cada motion y se confirma
        // en finishItem) — y el bridge XML+estilo llega con el modelo de
        // documento real.
        this->rect = SPRect::create();
        this->rect->transform = Geom::Affine();
        this->rect->updateRepr();
    }

    Geom::Rect const r = Inkscape::snap_rectangular_box(_desktop, rect.get(), pt, this->center, state);

    this->rect->setPosition(r.min()[Geom::X], r.min()[Geom::Y], r.dimensions()[Geom::X], r.dimensions()[Geom::Y]);

    if (this->rx != 0.0) {
        this->rect->setRx(true, this->rx);
    }

    if (this->ry != 0.0) {
        if (this->rx == 0.0) {
            this->rect->setRy(true, CLAMP(this->ry, 0, MIN(r.dimensions()[Geom::X], r.dimensions()[Geom::Y])/2));
        } else {
            this->rect->setRy(true, CLAMP(this->ry, 0, r.dimensions()[Geom::Y]));
        }
    }

    // status text
    double rdimx = r.dimensions()[Geom::X];
    double rdimy = r.dimensions()[Geom::Y];

    Inkscape::Util::Quantity rdimx_q = Inkscape::Util::Quantity(rdimx, "px");
    Inkscape::Util::Quantity rdimy_q = Inkscape::Util::Quantity(rdimy, "px");
    Glib::ustring xs = rdimx_q.string(_desktop->getNamedView()->display_units);
    Glib::ustring ys = rdimy_q.string(_desktop->getNamedView()->display_units);

    if (mod_trans_confine->active(state)) {
        int ratio_x, ratio_y;
        bool is_golden_ratio = false;

        if (fabs (rdimx) > fabs (rdimy)) {
            if (fabs(rdimx / rdimy - goldenratio) < 1e-6) {
                is_golden_ratio = true;
            }

            ratio_x = (int) rint (rdimx / rdimy);
            ratio_y = 1;
        } else {
            if (fabs(rdimy / rdimx - goldenratio) < 1e-6) {
                is_golden_ratio = true;
            }

            ratio_x = 1;
            ratio_y = (int) rint (rdimy / rdimx);
        }

        if (!is_golden_ratio) {
            this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE,
                    _("<b>Rectangle</b>: %s &#215; %s (constrained to ratio %d:%d); with <b>%s</b> to draw around the starting point"),
                    xs.c_str(), ys.c_str(), ratio_x, ratio_y,
                    mod_trans_off_center->get_label().c_str());
        } else {
            if (ratio_y == 1) {
                this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE,
                        _("<b>Rectangle</b>: %s &#215; %s (constrained to golden ratio 1.618 : 1); with <b>%s</b> to draw around the starting point"),
                        xs.c_str(), ys.c_str(), mod_trans_off_center->get_label().c_str());
            } else {
                this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE,
                        _("<b>Rectangle</b>: %s &#215; %s (constrained to golden ratio 1 : 1.618); with <b>%s</b> to draw around the starting point"),
                        xs.c_str(), ys.c_str(), mod_trans_off_center->get_label().c_str());
            }
        }
    } else {
        this->message_context->setF(Inkscape::IMMEDIATE_MESSAGE,
                _("<b>Rectangle</b>: %s &#215; %s; with <b>%s</b> to make square, integer-ratio, or golden-ratio rectangle; with <b>%s</b> to draw around the starting point"),
                xs.c_str(), ys.c_str(), mod_trans_confine->get_label().c_str(),
                mod_trans_off_center->get_label().c_str());
    }
}

void RectTool::finishItem() {
    this->message_context->clear();

    if (rect) {
        if (this->rect->width.computed == 0 || this->rect->height.computed == 0) {
            this->cancel(); // Don't allow the creating of zero sized rectangle, for example when the start and and point snap to the snap grid point
            return;
        }

        this->rect->updateRepr();
        this->rect->doWriteTransform(this->rect->transform, nullptr, true);
        // update while creating inside a LPE group
        sp_lpe_item_update_patheffect(this->rect.get(), true, true);
        _desktop->getSelection()->set(rect.get());

        DocumentUndo::done(_desktop->getDocument(), RC_("Undo", "Create rectangle"), INKSCAPE_ICON("draw-rectangle"));

        this->rect = nullptr;
    }
}

void RectTool::cancel(){
    _desktop->getSelection()->clear();
    ungrabCanvasEvents();

    if (rect) {
        rect->deleteObject();
    }

    this->within_tolerance = false;
    xyp = {};
    this->item_to_select = nullptr;

    DocumentUndo::cancel(_desktop->getDocument());
}

} // namespace Inkscape::UI::Tools