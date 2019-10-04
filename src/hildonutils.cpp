#include "hildonutils.h"
#include <QPainter>

HildonUtils::HildonUtils(QObject *parent) :
    QObject(parent)
{
}

int HildonUtils::currentView()
{
    int currentDesktop = X11Utils::getCurrentDesktop();

    if(isPortrait)
    {
        if(portraitWallpaper())
            return 10 + currentDesktop;
        else return currentDesktop + 1;
    }
    else return currentDesktop + 1;
}

bool HildonUtils::portraitWallpaper()
{
#ifdef Q_WS_MAEMO_5
    GConfClient *gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));

    QString spath = "/apps/osso/hildon-desktop/portrait_wallpaper";
    QByteArray barray = spath.toUtf8();
    const gchar *path = barray.constData();

    GConfValue *gcValue = NULL;
    try
    {
        gcValue = gconf_client_get_without_default(gconfClient, path, NULL);
    }
    catch(std::exception) {
        qDebug() << "exception /apps/osso/hildon-desktop/portrait_wallpaper";
        return false;
    }

    bool result = false;

    if(gcValue != NULL)
    {
        result = gconf_value_get_bool(gcValue);
        gconf_value_free(gcValue);
    }
    if(gconfClient)
        g_object_unref(gconfClient);

    return result;
#else
    return false;
#endif
}

void HildonUtils::lockOrientation()
{
#ifdef Q_WS_MAEMO_5
    GConfClient *gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));
    gconf_client_set_bool(gconfClient, "/apps/osso/hildon-desktop/orientation_lock", true, NULL);
    gconf_client_set_bool(gconfClient, "/apps/osso/hildon-desktop/ui_can_rotate", false, NULL);
    if(gconfClient)
        g_object_unref(gconfClient);
#endif
}

void HildonUtils::unLockOrientation()
{
#ifdef Q_WS_MAEMO_5
    GConfClient *gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));
    gconf_client_set_bool(gconfClient, "/apps/osso/hildon-desktop/orientation_lock", false, NULL);
    gconf_client_set_bool(gconfClient, "/apps/osso/hildon-desktop/ui_can_rotate", true, NULL);
    if(gconfClient)
        g_object_unref(gconfClient);
#endif
}

void HildonUtils::getOrientation(bool &lock, bool &canRotate)
{
#ifdef Q_WS_MAEMO_5
    GConfClient *gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));
    lock = gconf_client_get_bool(gconfClient, "/apps/osso/hildon-desktop/orientation_lock", NULL);
    canRotate = gconf_client_get_bool(gconfClient, "/apps/osso/hildon-desktop/ui_can_rotate", NULL);
    if(gconfClient)
        g_object_unref(gconfClient);
#endif
}

void HildonUtils::setOrientation(bool _lock, bool _canRotate)
{
#ifdef Q_WS_MAEMO_5
    GConfClient *gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));
    gconf_client_set_bool(gconfClient, "/apps/osso/hildon-desktop/orientation_lock", _lock, NULL);
    gconf_client_set_bool(gconfClient, "/apps/osso/hildon-desktop/ui_can_rotate", _canRotate, NULL);
    if(gconfClient)
        g_object_unref(gconfClient);
#endif
}

QPixmap HildonUtils::hildonHomeWallpaper()
{
#ifdef Q_WS_MAEMO_5
    GConfClient *gconfClient = gconf_client_get_default();
    g_assert(GCONF_IS_CLIENT(gconfClient));

    QString spath = QString("/apps/osso/hildon-desktop/views/%1/bg-image").arg(currentView());
    QByteArray barray = spath.toUtf8();
    const gchar *path = barray.constData();

    GConfValue *gcValue = NULL;
    try {
        gcValue = gconf_client_get_without_default(gconfClient, path, NULL);
    } catch(std::exception) {
        qDebug() << "change your wallpaper";
        return QPixmap();
    }

    if(gcValue == NULL)
        return QPixmap(800, 480);

    QString imagePath = gconf_value_get_string(gcValue);

    gconf_value_free(gcValue);

    if(gconfClient)
        g_object_unref(gconfClient);

    if(imagePath != NULL)
        return QPixmap(imagePath);
    else return QPixmap();
#else
    return QPixmap();
#endif
}

QList<Window> HildonUtils::_appletList()
{
    isDesktopDetect = false;
    appletList.clear();
    _windowList();

    return appletList;
}

Window HildonUtils::_hildonStatusArea()
{
    return hildonStatusArea;
}

void HildonUtils::_windowList()
{
    //this can be a huge list even > 300, hildon widgets are at the end of this list
    QList<Window> wlist = X11Utils::getWindowTree(QX11Info::appRootWindow(0));

    for (int i = wlist.count()-1; i >= 0; --i)
    {
        Atom wtype = X11Utils::getWindowType(wlist.at(i));
        if(wtype == X11Utils::returnAtom("_HILDON_WM_WINDOW_TYPE_STATUS_AREA"))
        {
            hildonStatusArea = wlist.at(i);
        }
        else if(wtype == X11Utils::returnAtom("_HILDON_WM_WINDOW_TYPE_HOME_APPLET"))
        {
            appletList.append(wlist.at(i));
        }
        else if(wtype == X11Utils::returnAtom("_NET_WM_WINDOW_TYPE_DESKTOP"))
        {
            return;
        }
    }
}

QImage HildonUtils::hildonHomeImage(QList<Window> appletList, bool isPortrait, Window hildonStatusArea)
{
    QPixmap pixmap;
    Display *dpy = QX11Info::display();


    pixmap = hildonHomeWallpaper();

    if(pixmap.width() > pixmap.height() && pixmap.size() != QSize(800, 480))
    {
        pixmap = pixmap.scaledToHeight(480, Qt::FastTransformation);
        pixmap = pixmap.copy((pixmap.width()-800)/2, 0, 800, 480);
    }
    else
    {
        pixmap = pixmap.scaledToWidth(800, Qt::FastTransformation);
        pixmap = pixmap.copy(0, (pixmap.height()-480)/2, 800, 480);
    }

    if(isPortrait)
    {
            QTransform iTransform;
            iTransform.rotate(90);
            pixmap = pixmap.transformed(iTransform);
    }

    QPainter painter(&pixmap);
    try
    {
        for (int i = appletList.count() - 1; i >= 0; --i)
        {
             XWindowAttributes attr;
             XGetWindowAttributes(dpy, appletList.at(i), &attr);
             painter.drawPixmap(attr.x,
                                attr.y,
                                X11Utils::getWindowPixmap((int)appletList.at(i), 0));
        }
    }
    catch(std::bad_alloc) {}

    painter.end();

    if(!pixmap.isNull())
        return pixmap.toImage();
    else return QImage();
}

QImage HildonUtils::getHildonHome(bool isPortrait)
{
    this->isPortrait = isPortrait;
    QList<Window> appletList = _appletList();
    Window hildonStatusArea = _hildonStatusArea();
    QImage image = hildonHomeImage(appletList, isPortrait, hildonStatusArea);
    appletList.clear();
    return image;
}
