// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * undo.h — stub of Inkscape::DocumentUndo.
 *
 * The alpha has no undo stack yet; done()/cancel() are no-ops. Undo/redo
 * arrive with the real document model phase.
 */
#ifndef SHAM_UNDO_H
#define SHAM_UNDO_H

class SPDocument;

namespace Inkscape {

class DocumentUndo {
public:
    static void done(SPDocument *, char const * /*description*/, char const * /*icon*/) {}
    static void cancel(SPDocument *) {}
};

} // namespace Inkscape

#endif // SHAM_UNDO_H