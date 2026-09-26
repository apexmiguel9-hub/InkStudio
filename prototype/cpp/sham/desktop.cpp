// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * desktop.cpp — construction of the alpha's SPDesktop stub.
 */
#include "desktop.h"
#include "selection.h"

SPDesktop::SPDesktop()
{
    namedview = new Inkscape::SPNamedView();
    snapindicator = new SnapIndicator();
    document = new SPDocument();
}

SPDesktop::~SPDesktop()
{
    delete namedview;
    delete snapindicator;
    delete document;
}

Inkscape::Selection *SPDesktop::getSelection() const
{
    // Alpha: a single selection singleton per process (no multi-desktop yet).
    static Inkscape::Selection sel;
    return &sel;
}