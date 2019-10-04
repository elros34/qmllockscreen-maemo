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
// Copyright (c) 2013, MadFishTheOne
// Copyright (c) 2019, elros34

#include "x11utils.h"
#include <QDebug>

X11Utils *X11Utils::x11Utils = NULL;

X11Utils::X11Utils(QObject *parent) :
    QObject(parent)
{
     x11Utils = this;

     dpy = QX11Info::display();

     x11Utils->_MB_CURRENT_APP_WINDOW = XInternAtom(QX11Info::display(), "_MB_CURRENT_APP_WINDOW", false);
     x11Utils->_NET_ACTIVE_WINDOW = XInternAtom(QX11Info::display(), "_NET_ACTIVE_WINDOW", false);
}

Atom X11Utils::returnAtom(const QString &name)
{
    if(!x11Utils->_atomlist.contains(name))
        x11Utils->_atomlist[name] = XInternAtom(x11Utils->dpy, name.toLatin1().data(), false);
    return x11Utils->_atomlist[name];
}
QPixmap X11Utils::getWindowPixmap(int w, int redirectW)
{
    if(redirectW)
    XCompositeRedirectWindow(QX11Info::display(), w, CompositeRedirectAutomatic);

    XFreePixmap(QX11Info::display(), x11Utils->xpixmap);

    x11Utils->xpixmap = XCompositeNameWindowPixmap(QX11Info::display(), w);

    QPixmap pixmap = QPixmap::fromX11Pixmap(x11Utils->xpixmap);

    if(redirectW)
        XCompositeUnredirectWindow(QX11Info::display(), w, CompositeRedirectAutomatic);

    return pixmap;
}

void X11Utils::setWindowAtoms(const Window &wId)
{
    int one = 1;
    XChangeProperty(QX11Info::display(), wId,
                    x11Utils->returnAtom("_HILDON_WM_ACTION_NO_TRANSITIONS"),
                    XA_CARDINAL, 32, PropModeReplace, (uchar*)&one, 1);

    quint32 ten = {10};
    XChangeProperty(QX11Info::display(), wId,
                    x11Utils->returnAtom("_HILDON_STACKING_LAYER"),
                    XA_CARDINAL, 32, PropModeReplace, (uchar*)&ten, 1);

    one = 1;
    XChangeProperty(QX11Info::display(), wId,
                    x11Utils->returnAtom("_HILDON_DO_NOT_DISTURB"),
                    XA_INTEGER, 32, PropModeReplace, (uchar*)&one, 1);

    XChangeProperty(QX11Info::display(), wId,
                    x11Utils->returnAtom("_HILDON_NON_COMPOSITED_WINDOW"),
                    XA_INTEGER, 32, PropModeReplace, (uchar*)&one, 1);


    QVector<Atom> stateprop;
    stateprop.append(x11Utils->returnAtom("_NET_WM_STATE_SKIP_TASKBAR"));
    stateprop.append(x11Utils->returnAtom("_NET_WM_STATE_ABOVE"));
#ifdef Q_WS_MAEMO_5
    stateprop.append(x11Utils->returnAtom("_NET_WM_STATE_FULLSCREEN"));
#endif
    XChangeProperty(QX11Info::display(), wId, x11Utils->returnAtom("_NET_WM_STATE"),
                    XA_ATOM, 32, PropModeReplace,
                    (unsigned char*)stateprop.data(), stateprop.count());
}

QList<Window> X11Utils::getWindowListStacking()
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(0), returnAtom("_NET_CLIENT_LIST_STACKING"),
                          0, LONG_MAX, false, XA_WINDOW, &type, &format, &n, &remain, &data) != Success)
        return QList<Window>();

    if(data != NULL)
    {
        QList<Window> wlist;
        for (uint i = 0; i < n; ++i)
            wlist.append(((Window*)data)[i]);

        XFree(data);

        return wlist;
    }
    else return QList<Window>();
}

QList<Window> X11Utils::getWindowList()
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(0), returnAtom("_NET_CLIENT_LIST"),
                          0, LONG_MAX, false, XA_WINDOW, &type, &format, &n, &remain, &data) != Success)
        return QList<Window>();

    if(data != NULL)
    {
        QList<Window> wlist;
        for (uint i = 0; i < n; ++i)
            wlist.append(((Window*)data)[i]);

        XFree(data);

        return wlist;
    }
    else return QList<Window>();
}

void X11Utils::activateWId(Window &wId)
{
    XEvent xev;
    xev.type = ClientMessage;
    xev.xclient.window = wId;
    xev.xclient.message_type = returnAtom("_NET_ACTIVE_WINDOW");
    xev.xclient.format = 32;
    xev.xclient.data.l[1] = 1;
    xev.xclient.data.l[1] = QX11Info::appUserTime();

    XSendEvent(QX11Info::display(), QX11Info::appRootWindow(0), false,
               SubstructureRedirectMask, &xev);
}

QString X11Utils::getWindowName(const Window &wId)
{
    XTextProperty textProperty;
    QString wmName;
    XGetTextProperty(QX11Info::display(), wId, &textProperty, returnAtom("_NET_WM_NAME"));
    if(!textProperty.value)
    {
        XGetTextProperty(QX11Info::display(), wId, &textProperty, returnAtom("WM_NAME"));
    }
    if(textProperty.value)
    {
        wmName = QString((char*)textProperty.value);
        XFree(textProperty.value);
    }

    return wmName;
}

QString X11Utils::getWindowClass(const Window &wId)
{
    XTextProperty textProperty;
    QString wmClass;
    XGetTextProperty(QX11Info::display(), wId, &textProperty, returnAtom("WM_CLASS"));

    if(textProperty.value)
    {
        wmClass = QString((char*)textProperty.value);
        XFree(textProperty.value);
    }

    return wmClass;
}

QList<Atom> X11Utils::getWindowTypeList(const Window &wId)
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), wId, returnAtom("_NET_WM_WINDOW_TYPE"),
                          0, LONG_MAX, false, XA_ATOM, &type, &format, &n, &remain, &data) != Success)
        return QList<Atom>();

    if(data != NULL)
    {
        QList<Atom> wlist;
        for (uint i = 0; i < n; ++i)
            wlist.append(((Atom*)data)[i]);

        XFree(data);

        return wlist;
    }
    else return QList<Atom>();
}

Atom X11Utils::getWindowType(const Window &wId)
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), wId, returnAtom("_NET_WM_WINDOW_TYPE"),
                          0, LONG_MAX, false, XA_ATOM, &type, &format, &n, &remain, &data) != Success)
        return Atom();

    if(data != NULL)
    {
        Atom wtype = ((Atom*)data)[0];

        XFree(data);

        return wtype;
    }
    else return Atom();
}

QList<Atom> X11Utils::getWindowNState(const Window &wId)
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), wId, returnAtom("_NET_WM_STATE"),
                          0, LONG_MAX, false, XA_ATOM, &type, &format, &n, &remain, &data) != Success)
        return QList<Atom>();

    if(data != NULL)
    {
        QList<Atom> wlist;
        for (uint i = 0; i < n; ++i)
            wlist.append(((Atom*)data)[i]);

        XFree(data);

        return wlist;
    }
    else return QList<Atom>();
}

QList<Atom> X11Utils::getWindowState(const Window &wId)
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), wId, returnAtom("WM_STATE"),
                          0, LONG_MAX, false, returnAtom("WM_STATE"),
                          &type, &format, &n, &remain, &data) != Success)
        return QList<Atom>();

    if(data != NULL)
    {
        QList<Atom> wlist;
        for (uint i = 0; i < n; ++i)
            wlist.append(((Atom*)data)[i]);

        XFree(data);

        return wlist;
    }
    else return QList<Atom>();
}

Window X11Utils::getActiveWId()
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(0), returnAtom("_NET_ACTIVE_WINDOW"),
                          0, LONG_MAX, false, XA_WINDOW, &type, &format, &n, &remain, &data) != Success)
        return Window(-1);

    if(data != NULL)
    {
        Window wId = ((Window*)data)[0];

        XFree(data);

        return wId;
    }
    else return Window(-1);
}
void X11Utils::getActiveWId(Window &wId)
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(0), returnAtom("_NET_ACTIVE_WINDOW"),
                          0, LONG_MAX, false, XA_WINDOW, &type, &format, &n, &remain, &data) != Success)
        return;

    if(data != NULL)
    {
        wId = ((Window*)data)[0];

        XFree(data);

        return;
    }
    else return;
}

Window X11Utils::getMBActiveWId()
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(0), returnAtom("_MB_CURRENT_APP_WINDOW"),
                          0, LONG_MAX, false, XA_WINDOW, &type, &format, &n, &remain, &data) != Success)
        return Window(-1);

    if(data != NULL)
    {
        Window wId = ((Window*)data)[0];

        XFree(data);

        return wId;
    }
    else return Window(-1); // -1 is task manager
}
void X11Utils::getMBActiveWId(Window &wId)
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(0), returnAtom("_MB_CURRENT_APP_WINDOW"),
                          0, LONG_MAX, false, XA_WINDOW, &type, &format, &n, &remain, &data) != Success)
        return;

    if(data != NULL)
    {
        wId = ((Window*)data)[0];

        XFree(data);

        return;
    }
    else return; // -1 is task manager
}

bool X11Utils::isFullScreen(const Window &wId)
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), wId, returnAtom("_NET_WM_STATE"),
                          0, LONG_MAX, false, XA_ATOM, &type, &format, &n, &remain, &data) != Success)
        return false;

    if(data != NULL)
    {
        bool result = false;

        for (uint i = 0; i < n; ++i)
        {
            if(((Atom*)data)[i] ==  returnAtom("_NET_WM_STATE_FULLSCREEN"))
            {
                result = true;
                break;
            }
        }

        XFree(data);

        return result;
    }
    else return false;
}

QString X11Utils::getWindowOrientationSupport(const Window &wId)
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *wdata = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), wId, returnAtom("_HILDON_PORTRAIT_MODE_SUPPORT"),
                          0, LONG_MAX, false, XA_CARDINAL, &type, &format, &n , &remain, &wdata) != Success)
        return QString();

    if(wdata != NULL)
    {
        int result = ((int*)wdata)[0];
        XFree(wdata);

        if(result == 1)
            return QString("auto");
        else
            return QString("landscape");
    }
    else return QString("empty");
}

void X11Utils::changeWindowOrientationSupport(const Window &wId, const QString &name)
{
    if(name == "empty")
    {
        XDeleteProperty(QX11Info::display(), wId, x11Utils->returnAtom("_HILDON_PORTRAIT_MODE_SUPPORT"));
    }
    else if(name == "auto" || name == "landscape")
    {
        int one;
        if(name == "auto")
            one = 1;
        else if(name == "landscape")
            one = 0;

    XChangeProperty(QX11Info::display(), wId,
                   x11Utils->returnAtom("_HILDON_PORTRAIT_MODE_SUPPORT"),
                   XA_CARDINAL, 32, PropModeReplace, (uchar*)&one, 1);
    }
}

QString X11Utils::getWindowOrientationRequest(const Window &wId)
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *wdata = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), wId, returnAtom("_HILDON_PORTRAIT_MODE_REQUEST"),
                          0, LONG_MAX, false, XA_CARDINAL, &type, &format, &n , &remain, &wdata) != Success)
        return QString();

    if(wdata != NULL)
    {
        int result = ((int*)wdata)[0];
        XFree(wdata);

        if(result == 1)
            return QString("enable");
        else
            return QString("disable");
    }
    else return QString("empty");
}

void X11Utils::changeWindowOrientationPRequest(const Window &wId, const QString &name)
{
    if(name == "empty")
    {
        XDeleteProperty(QX11Info::display(), wId, x11Utils->returnAtom("_HILDON_PORTRAIT_MODE_REQUEST"));
    }
    else if(name == "enable" || name == "disable")
    {
        int one;
        if(name == "enable")
            one = 1;
        else if(name == "disable")
            one = 0;

    XChangeProperty(QX11Info::display(), wId,
                   x11Utils->returnAtom("_HILDON_PORTRAIT_MODE_REQUEST"),
                   XA_CARDINAL, 32, PropModeReplace, (uchar*)&one, 1);
    }
}

void X11Utils::changeWindowComposition(const Window &wId, const QString &name)
{
    if(name == "empty")
    {
        XDeleteProperty(QX11Info::display(), wId, x11Utils->returnAtom("_HILDON_NON_COMPOSITED_WINDOW"));
    }
    else
    {
        int one;
        if(name == "disable")
            one = 1;
        else if(name == "enable")
            one = 0;
        XChangeProperty(QX11Info::display(), wId,
                       x11Utils->returnAtom("_HILDON_NON_COMPOSITED_WINDOW"),
                       XA_INTEGER, 32, PropModeReplace, (uchar*)&one, 1);
    }
}

QString X11Utils::getWindowComposition(const Window &wId)
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *wdata = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), wId, returnAtom("_HILDON_NON_COMPOSITED_WINDOW"),
                          0, LONG_MAX, false, XA_INTEGER, &type, &format, &n , &remain, &wdata) != Success)
        return QString();

    if(wdata != NULL)
    {
        int result = ((int*)wdata)[0];
        XFree(wdata);

        if(result == 1)
            return QString("disable");
        else QString("enable");
    }
    return QString("empty");
}

void X11Utils::setLockScreenFlag(const Window &wId, const int &prop)
{
    if(prop == 0)
    {
        XDeleteProperty(QX11Info::display(), wId, x11Utils->returnAtom("QML_LOCK_SCREEN"));
    }
    else
    {
        int one = 1;

    XChangeProperty(QX11Info::display(), wId,
                   x11Utils->returnAtom("QML_LOCK_SCREEN"),
                   XA_INTEGER, 32, PropModeReplace, (uchar*)&one, 1);
    }
}

int X11Utils::getLockScreenFlag(const Window &wId)
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *wdata = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), wId, returnAtom("QML_LOCK_SCREEN"),
                          0, LONG_MAX, false, XA_INTEGER, &type, &format, &n , &remain, &wdata) != Success)
        return 0;

    if(wdata != NULL)
    {
        int result = ((int*)wdata)[0];
        XFree(wdata);

        return result; // 1

    }
    else return 0;
}

QList<Window> X11Utils::getWindowTree(const Window &wId)
{
    Window root_return, parent_return, *windows;
    unsigned int nwindows;

    if(XQueryTree(QX11Info::display(), wId, &root_return, &parent_return, &windows, &nwindows) == 0)
        return QList<Window>();

    QList<Window> wlist;

    for (uint i = 0; i < nwindows; ++i)
        wlist.append(windows[i]);

    XFree(windows);

    return wlist;
}

bool X11Utils::getWindowPropertyHelper(const uchar data, const ulong &n) //char* &data
{

    return true;
}

QString X11Utils::getWindowNotificationType(const Window &wId)
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *wdata = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), wId, returnAtom("_HILDON_INCOMING_EVENT_NOTIFICATION_DESTINATION"),
                          0, LONG_MAX, false, XA_STRING, &type, &format, &n , &remain, &wdata) != Success)
        return QString();

    if(wdata != NULL)
    {
        QString result = (char*)wdata;
        XFree(wdata);

        return result;
    }
    return QString();
}

bool X11Utils::isDesktop()
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(0), returnAtom("_NET_SHOWING_DESKTOP"),
                          0, LONG_MAX, false, XA_CARDINAL, &type, &format, &n, &remain, &data) != Success)
        return false;

    if(data != NULL)
    {
        bool result = ((int*)data)[0];
        XFree(data);

        return result;
    }
    return false;
}

void X11Utils::show(const Window &wId)
{
    XEvent event;

    event.xclient.type = ClientMessage;
    event.xclient.send_event = True;
    event.xclient.message_type = x11Utils->_NET_ACTIVE_WINDOW;
    event.xclient.window = wId;
    event.xclient.format = 32;


    XSendEvent(x11Utils->dpy, DefaultRootWindow(x11Utils->dpy), False,
            SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XMapRaised(x11Utils->dpy, wId);
}

int X11Utils::getCurrentDesktop()
{
    Atom type;
    ulong n;
    ulong remain;
    uchar *data = NULL;
    int format;

    if(XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(0), returnAtom("_NET_CURRENT_DESKTOP"),
                          0, LONG_MAX, false, XA_CARDINAL, &type, &format, &n, &remain, &data) != Success)
        return -1;

    if(data != NULL)
    {
        int num = ((int*)data)[0];

        XFree(data);

        return num;
    }
    else return -1;
}
