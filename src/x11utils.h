#ifndef X11UTILS_H
#define X11UTILS_H

#include <QObject>
#include <QMap>
#include <QTextStream>
#include <QX11Info>
#include <QPixmap>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>

class X11Utils : public QObject
{
    Q_OBJECT
public:
    explicit X11Utils(QObject *parent = 0);

    static int myXErrorHandler( Display*, XErrorEvent*) {
        return 1;
    }

    Display *dpy;

    Atom _MB_CURRENT_APP_WINDOW;
    Atom _NET_ACTIVE_WINDOW;

private:
    static X11Utils *x11Utils;
    QMap<QString, Atom> _atomlist;

    Pixmap xpixmap;
    
signals:
    
public slots:
    static Atom returnAtom(const QString &name);
    static QPixmap getWindowPixmap(int w, int redirectW);
    static void setWindowAtoms(const Window &wId);
    static QList<Window> getWindowListStacking();
    static QList<Window> getWindowList();
    static void activateWId(Window &wId);
    static QString getWindowName(const Window &wId);
    static QString getWindowClass(const Window &wId);
    static QList<Atom> getWindowTypeList(const Window &wId);
    static Atom getWindowType(const Window &wId);
    static QList<Atom> getWindowNState(const Window &wId);
    static QList<Atom> getWindowState(const Window &wId);
    static Window getActiveWId();
    static void getActiveWId(Window &wId);
    static Window getMBActiveWId();
    static void getMBActiveWId(Window &wId);
    static bool isFullScreen(const Window &wId);
    static QString getWindowOrientationSupport(const Window &wId);
    static void changeWindowOrientationSupport(const Window &wId, const QString &name);
    static QString getWindowOrientationRequest(const Window &wId);
    static void changeWindowOrientationPRequest(const Window &wId, const QString &name);
    static void changeWindowComposition(const Window &wId, const QString &name);
    static QString getWindowComposition(const Window &wId);
    static void setLockScreenFlag(const Window &wId, const int &prop);
    static int getLockScreenFlag(const Window &wId);
    static QList<Window> getWindowTree(const Window &wId);
    static bool getWindowPropertyHelper(const uchar data, const ulong &n);
    static QString getWindowNotificationType(const Window &wId);
    static bool isDesktop();
    static void show(const Window &wId);
    static int getCurrentDesktop();
    
};

#endif // X11UTILS_H
