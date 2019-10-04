#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "qtextstream.h"
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QtDBus>

#include "imageprovider.h"
#include "filemodel.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xcomposite.h>

#include <QX11Info>

class MainView : public QDeclarativeView
{
    Q_OBJECT
public:
    explicit MainView(QWidget *parent = 0);
    ~MainView();

    void changeEvent(QEvent *event);
    void hideEvent(QHideEvent *);

    Window statusArea;
    Window statusMenu;
    ImageProvider *imageProvider;
    bool isPortrait;
    bool _isVisible;

private:
    Atom _HILDON_WM_WINDOW_TYPE_STATUS_AREA;
    Atom _HILDON_WM_WINDOW_TYPE_STATUS_MENU;
    Atom _NET_WM_WINDOW_TYPE;
    bool isPressed;
    bool isKeyboardOpen;
    QString pendingOrientation;
    QDBusMessage _minimize;

signals:
    void updateStatusArea(int wId);

public slots:
    void _show();
    void minimize();
    void onUpdateStatusArea();
    void onImageReady();
    void moveStatusArea();

private slots:
    void onOrientationChanged();
    void onShowEvent();
    void findStatusWindows();
    Atom getWindowType(const Window &wId);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void onDBusOrientationChanged(const QString &orientation);
    void onKeyboardSlideChanged();

};

#endif // MAINVIEW_H
