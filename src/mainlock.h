#ifndef MAINLOCK_H
#define MAINLOCK_H

#include <QDeclarativeView>
#include <QtGui>
#include <QDeclarativeContext>
#include <QtDBus>
#include <QDebug>
#include "windowitem.h"
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>

#include "x11utils.h"
#include "uielements.h"
#include "Settings.h"

#include <QShowEvent>
#include <QEvent>
#include <QtConcurrentRun>

class MainLock : public QDeclarativeView
{
    Q_OBJECT
public:
    explicit MainLock(QWidget *parent = 0);

    bool isLocked;

private:
    bool isHildon;
    bool isInactivity;
    bool doNotHide;
    bool isPortrait;
    bool slideUnlock;
    bool isAlreadyLocked;
    bool isHildonWindow;
    bool callActive;
    bool wasCallActive;

    QString displayStatus;

    X11Utils *x11Utils;
    UiElements *uiElements;

    Window bottomWId;
    Window latestActive;
    Window hildon_desktop;

    struct WindowInfo {
        Window wId;
        QString wname;
        QString wclass;
        bool isFullScreen;
        QString worientationSupport;
        QString worientationPRequest;
        QString wcomposition;
    };

    WindowInfo winfo;

    bool hildon_lock;
    bool hildon_canRotate;

    
signals:
    void updateBottomView(QString wId, int isFWId);
    void realShowSignal();
    void mouseReleaseSignal();
    void firstStart();
public slots:
    void hideLockScreen();
    void onX11Event(XEvent *event);

    void onSetMouseEvents(bool enable);

    void mouseAction(const int &x, const int &y, const bool &press);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void onLockChanged(QString msg);
    void onPowerButtonPressed();
    void onIncommingCall();
    void onTerminatedCall();
    void onAlarmStatusChanged();
    void onDisplayStatusChanged(const QString &state);
    void onInactivity_ind(bool state);
    void onOrientationChanged();
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void updateWindowView();
    void onOffLockedState();
    void updateDesktopView();
    void checkHildonHome();
    void onMissedCall(const QDBusMessage &msg);
};

#endif // MAINLOCK_H
