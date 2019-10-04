#ifndef HILDONUTILS_H
#define HILDONUTILS_H

#include <QObject>
#include <QtGui>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDebug>
#include <gconf/gconf-client.h>
#include <QImage>
#include <QX11Info>
#include "x11utils.h"
#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include "X11/extensions/Xrender.h"

class HildonUtils : public QObject
{
    Q_OBJECT
public:
    explicit HildonUtils(QObject *parent = 0);

    QList<Window> appletList;
    Window hildonStatusArea;

    bool isDesktopDetect;

private:
    bool isPortrait;
    bool isPortraitWallpaper;
signals:

private slots:
    int currentView();
    QPixmap hildonHomeWallpaper();
    QList<Window> _appletList();
    Window _hildonStatusArea();
    void _windowList();
    bool portraitWallpaper();



public slots:
    QImage hildonHomeImage(QList<Window> appletList, bool isPortrait, Window hildonStatusArea);
    QImage getHildonHome(bool isPortrait);

    static void lockOrientation();
    static void unLockOrientation();
    static void getOrientation(bool &lock, bool &canRotate);
    static void setOrientation(bool _lock, bool _canRotate);

};

#endif // HILDONUTILS_H
