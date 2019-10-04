// This program or library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation
// and appearing in the file LICENSE.LGPL included in the packaging
// of this file.
//
// This code is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// Copyright (c) 2011, Robin Burchell
// Copyright (c) 2012, Timur Kristóf <venemo@fedoraproject.org>
// Copyright (c) 2010, Nokia Corporation and/or its subsidiary(-ies) <directui@nokia.com>
// Copyright (c) 2019, elros34

#include "x11helper.h"
#include <QDebug>

X11Helper::X11Helper(QObject *parent) :
    QObject(parent)
{
}

QPixmap X11Helper::getWindowPixmap(const uint wId)
{
    XFreePixmap(QX11Info::display(), xpixmap);
    XCompositeRedirectWindow(QX11Info::display(), wId, CompositeRedirectAutomatic);
    xpixmap = XCompositeNameWindowPixmap(QX11Info::display(), wId);
    return QPixmap::fromX11Pixmap(xpixmap, QPixmap::ExplicitlyShared);
}
