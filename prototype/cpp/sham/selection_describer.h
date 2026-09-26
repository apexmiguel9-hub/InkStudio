// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * selection_describer.h — stub of Inkscape's SelectionDescriber.
 *
 * In Inkscape this prints the current selection's properties to the status
 * bar ("N objects selected…"). The alpha stores the message in the same
 * MessageStack the tools use; a future HUD can draw it.
 */
#ifndef SHAM_SELECTION_DESCRIBER_H
#define SHAM_SELECTION_DESCRIBER_H

#include "message.h" // MessageStack (forward-declared surface)

namespace Inkscape {

class Selection;

class SelectionDescriber {
public:
    SelectionDescriber(Selection *, MessageStack &, char const * /*tip*/,
                       char const * /*no_selection_msg*/)
    {}
    void updateMessage(Selection *) {}
};

} // namespace Inkscape

#endif // SHAM_SELECTION_DESCRIBER_H