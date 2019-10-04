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


#include "mainlock.h"

MainLock::MainLock(QWidget *parent) :
    QDeclarativeView(parent)
{
    x11Utils = new X11Utils(this);
    uiElements = new UiElements(this);

    connect(this, SIGNAL(realShowSignal()), uiElements, SLOT(onRealShowSignal()));

#ifdef Q_WS_MAEMO_5
    isHildon = true;
#endif

    if(QApplication::desktop()->height() > QApplication::desktop()->width())
        isPortrait = true;
    else isPortrait = false;

    QDBusConnection::systemBus().connect("",
                                         "/com/nokia/mce/signal",
                                         "com.nokia.mce.signal",
                                         "tklock_mode_ind",
                                         this,
                                         SLOT(onLockChanged(QString)));
    QDBusConnection::systemBus().connect("org.freedesktop.Hal",
                                         "/org/freedesktop/Hal/devices/computer_logicaldev_input",
                                         "org.freedesktop.Hal.Device",
                                         "Condition", this, SLOT(onPowerButtonPressed()));
    QDBusConnection::systemBus().connect(QString(),
                                         QString(),
                                         "com.nokia.mce.signal",
                                         "display_status_ind",
                                         this, SLOT(onDisplayStatusChanged(const QString &)));
    QDBusConnection::systemBus().connect(QString(),
                                         QString(),
                                         "com.nokia.mce.signal",
                                         "system_inactivity_ind",
                                         this, SLOT(onDisplayStatusChanged(const QString &)));
    QDBusConnection::systemBus().connect(QString(),
                                         QString(),
                                         "com.nokia.csd.Call",
                                         "Coming",
                                         this, SLOT(onIncommingCall()));
    QDBusConnection::systemBus().connect("com.nokia.csd.Call",
                                         "/com/nokia/csd/call/1",
                                         "com.nokia.csd.Call.Instance",
                                         "Terminated", this, SLOT(onTerminatedCall()));

    QDBusConnection::systemBus().connect(QString(),
                                         QString(),
                                         "com.nokia.systemui",
                                         "alarm_dialog_status",
                                         this, SLOT(onAlarmStatusChanged()));

    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                          "/org/freedesktop/Notifications",
                                          "org.freedesktop.Notifications",
                                          "NotificationClosed", this,
                                          SLOT(onMissedCall(const QDBusMessage &)));

    setWindowFlags(Qt::WindowStaysOnTopHint);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5AutoOrientation, true);
#else
    setAttribute(Qt::WA_AutoOrientation);
#endif

    X11Utils::setWindowAtoms(this->winId());

    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    viewport()->setAttribute(Qt::WA_NoSystemBackground);
    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    setResizeMode(QDeclarativeView::SizeRootObjectToView);
    setAutoFillBackground(true);

    rootContext()->setContextProperty("appPath", QCoreApplication::applicationDirPath());
    rootContext()->setContextProperty("isPortrait", isPortrait);
    rootContext()->setContextProperty("mainLock", this);
    rootContext()->setContextProperty("wId", QString());
    rootContext()->setContextProperty("isHildon", isHildon);
    rootContext()->setContextProperty("uiTools", uiElements);

    Settings *settings = new Settings(this);
    rootContext()->setContextProperty("Settings", settings);

    qmlRegisterType<WindowItem>("Utils",1,0,"WindowItem");

    settings->beginGroup("lockscreen");
    QString theme = settings->value("theme", "vertical-v2").toString();
    settings->setValue("theme", theme);
    settings->endGroup();
    settings->sync();

    setSource(QUrl::fromLocalFile(QString::fromLatin1("/%1/qml/%2/main.qml").arg(
                                         QCoreApplication::applicationDirPath(), theme)));

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(onOrientationChanged()));

    isLocked = false;
    isInactivity = false;
    doNotHide = true;
    isPortrait = false;
    slideUnlock = false;
    isAlreadyLocked = false;
    isHildonWindow = false;
    wasCallActive = false;

    displayStatus = "on";

    latestActive = 0;
    callActive= false;

    qDebug() << this->winId();

#ifdef Q_WS_MAEMO_5
    QProcess::execute("gconftool-2 -s /system/systemui/tklock/auto_rotation -t bool false");
    QProcess::execute("sudo /opt/qmllockscreen/chmod_i2c.sh");
    checkHildonHome(); //check whether 0x1c00005 is hildon home
#else
    show();
#endif
}

void MainLock::mouseAction(const int &x, const int &y, const bool &press)
{
    QGraphicsSceneMouseEvent pressEvent(press ? QEvent::GraphicsSceneMousePress : QEvent::GraphicsSceneMouseRelease);
    pressEvent.setScenePos(QPointF(x, y));
    pressEvent.setButton(Qt::LeftButton);
    pressEvent.setButtons(Qt::LeftButton);

    QGraphicsItem* item = this->itemAt(x, y);
    qDebug() << "result: " << this->scene()->sendEvent(item, &pressEvent);
}

void MainLock::mouseMoveEvent(QMouseEvent *event)
{
    QDeclarativeView::mouseMoveEvent(event);
}

void MainLock::onLockChanged(QString msg)
{
    hide();

    isAlreadyLocked = false;

    if(msg == "locked")
    {
        if(isLocked)
        {
            isAlreadyLocked = true;
        }

        isLocked = true;
    }
    else if(msg == "unlocked")
    {
        isLocked = false;

        slideUnlock = false;

        uiElements->onOffLockedScreen();

        if(X11Utils::getLockScreenFlag(winfo.wId) == 1)
        {
            if(!isHildonWindow && winfo.isFullScreen)
                X11Utils::changeWindowComposition(winfo.wId, winfo.wcomposition);

            if(!callActive)
            {
                HildonUtils::setOrientation(hildon_lock, hildon_canRotate);
                X11Utils::setLockScreenFlag(winfo.wId, 0);
            }
            else
                wasCallActive = true;
        }
    }

    uiElements->isLocked = isLocked;
}

void MainLock::onDisplayStatusChanged(const QString &state)
{
    if(state == "on")
    {
        displayStatus = "on";
        if(isLocked && !callActive)
        {
            uiElements->onOnLockedScreen();
        }
    }
    else if(state == "off")
    {
        if(isHildon)
            hide();

        displayStatus = "off";
        if(!isAlreadyLocked && isLocked)
        {
            onOffLockedState();
            uiElements->onOffScreen();
        }
        if(isAlreadyLocked)
            uiElements->onOffLockedScreen();
    }
    else if (state == "dimmed")
    {
        displayStatus = "dimmed";
    }
}

void MainLock::onOffLockedState()
{
    if(wasCallActive)
    {
        wasCallActive = false;
        return;
    }

    Window wId = X11Utils::getMBActiveWId();

    if(wId == 0xffffffff || wId == 0x0 || wId == hildon_desktop)// task || app || home
    {
        isHildonWindow = true;

        wId = X11Utils::getActiveWId();
        winfo.wId = wId;

        updateDesktopView();
    }
    else {
        if(!isHildon)
            wId = X11Utils::getActiveWId();

        isHildonWindow = false;

        winfo.wId = wId;
        winfo.wname = X11Utils::getWindowName(wId);
        winfo.wclass = X11Utils::getWindowClass(wId);

        if(winfo.wclass == "Conky")
            winfo.isFullScreen  = true;
        else
            winfo.isFullScreen = X11Utils::isFullScreen(wId);

        winfo.worientationSupport = X11Utils::getWindowOrientationSupport(wId);
        winfo.worientationPRequest = X11Utils::getWindowOrientationRequest(wId);
        winfo.wcomposition = X11Utils::getWindowComposition(wId);
    }

    HildonUtils::getOrientation(hildon_lock, hildon_canRotate);
    if(isPortrait)
        HildonUtils::setOrientation(true, true);
    else
        HildonUtils::setOrientation(true, false);

    X11Utils::setLockScreenFlag(winfo.wId, 1);
}

void MainLock::onPowerButtonPressed()
{
    if(isLocked)
    {
        if(displayStatus == "on")
        {
            uiElements->sendLock();
        }
        else {
            if(isHildon)
            {
                if(winfo.isFullScreen && !isHildonWindow)
                    X11Utils::changeWindowComposition(winfo.wId, "enable");

                showFullScreen();
            }
            else
            {
                show();
            }
        }
    }
}

void MainLock::onIncommingCall()
{
    hide();
    callActive = true;
}

void MainLock::onTerminatedCall()
{
    callActive = false;
}

void MainLock::onMissedCall(const QDBusMessage &msg)
{
}

void MainLock::onAlarmStatusChanged()
{
    hide();
}

void MainLock::updateDesktopView()
{
    emit updateBottomView("desktop", 1);
}

void MainLock::onInactivity_ind(bool state)
{
    isInactivity = state;
}

void MainLock::onOrientationChanged()
{
    isPortrait = (QApplication::desktop()->height() > QApplication::desktop()->width()) ? true : false;
    rootContext()->setContextProperty("isPortrait", isPortrait);
}

void MainLock::showEvent(QShowEvent *)
{
    uiElements->onShowSignal();
   X11Utils::changeWindowOrientationSupport(this->winId(), winfo.worientationSupport);
   X11Utils::changeWindowOrientationPRequest(this->winId(), winfo.worientationPRequest);
}

void MainLock::hideEvent(QHideEvent *)
{
    if(!isHildonWindow &&
       X11Utils::getLockScreenFlag(winfo.wId) == 1)
    {
        if(winfo.isFullScreen)
        {
            X11Utils::changeWindowComposition(winfo.wId, winfo.wcomposition);
        }

    }
}

void MainLock::hideLockScreen()
{
    slideUnlock = true;
    uiElements->sendUnLock();
}

void MainLock::onX11Event(XEvent *event)
{
    if(isHildon)
    {
        if(event->type == PropertyNotify)
        {
            if(event->xproperty.atom == x11Utils->_MB_CURRENT_APP_WINDOW)
            {
                if(event->xproperty.window == this->winId())
                {
                    Window wId;

                    x11Utils->getMBActiveWId(wId);

                    if(wId == this->winId() &&
                       wId != latestActive)  //on show lockscreen
                    {
                        viewport()->update();
                        emit realShowSignal();
                        if(!isHildonWindow)
                        {
                            QTimer::singleShot(0, this, SLOT(updateWindowView()));
                        }
                    }
                    else if(latestActive == this->winId() &&
                            wId != this->winId())
                    {
                        X11Utils::changeWindowOrientationSupport(this->winId(), "auto");
                        X11Utils::changeWindowOrientationPRequest(this->winId(), "empty");
                    }
                    latestActive = wId;
                }
                else if(event->xproperty.window == QX11Info::appRootWindow(0))
                {
                    if(isLocked)
                    {
                        qDebug() << "screen locked, window changed!";
                    }
                }
            }
        }
    }
    else { //desktop pc
        if(event->type == PropertyNotify &&
           event->xproperty.window == this->winId() &&
           event->xproperty.atom == x11Utils->_NET_ACTIVE_WINDOW)
        {
            Window wId;

            x11Utils->getActiveWId(wId);

            if(wId == this->winId() &&
               wId != latestActive)  //on show lockscreen
            {
                if(!isHildonWindow)
                {
                    QTimer::singleShot(10, this, SLOT(updateWindowView()));
                }

                emit realShowSignal();
            }
            else if(latestActive == this->winId() &&
                    wId != this->winId())
            {
                qDebug() << "\nreal hide signal";
            }
            latestActive = wId;
        }
    }
}

void MainLock::onSetMouseEvents(bool enable)
{
    setEnabled(enable);
}

void MainLock::updateWindowView()
{
    emit updateBottomView(QString::number(winfo.wId),
                          (int)winfo.isFullScreen);
}

void MainLock::checkHildonHome()
{
    hildon_desktop = 0;

    if(!isHildon)
        return;

    QList<Window> wlist = X11Utils::getWindowList();
    for(int i = 0; i < wlist.count(); ++i)
    {
        if(X11Utils::getWindowType(wlist.at(i)) == X11Utils::returnAtom("_NET_WM_WINDOW_TYPE_DESKTOP") ||
           X11Utils::getWindowName(wlist.at(i)) == "desktop")
        {
             qDebug() << "desktop is: " << QString::number(wlist.at(i), 16);
             hildon_desktop = wlist.at(i);
             break;
        }
    }

    if(hildon_desktop == 0)
    {
        qDebug() << "cannot find _NET_WM_WINDOW_TYPE_DESKTOP type window";
        exit(1);
    }
}
