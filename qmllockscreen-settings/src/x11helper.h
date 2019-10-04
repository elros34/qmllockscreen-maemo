#ifndef X11HELPER_H
#define X11HELPER_H

#include <QObject>
#include <QPixmap>

#include "qtextstream.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xcomposite.h>
#include <QX11Info>


class X11Helper : public QObject
{
    Q_OBJECT
public:
    explicit X11Helper(QObject *parent = 0);

private:
    Pixmap xpixmap;


public slots:
    QPixmap getWindowPixmap(const uint wId);

};

#endif // X11HELPER_H
