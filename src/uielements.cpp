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
//
// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// Copyright (c) 2019 elros34

#include "uielements.h"
#include <QDebug>

#define MAFW_STATUS_STOPPED 0
#define MAFW_STATUS_STARTED 1
#define MAFW_STATUS_PAUSED 2
#define MAFW_STATUS_TRANSITION 3

#define QMLRADIO_STATUS_NULL 0
#define QMLRADIO_STATUS_STOPED 1
#define QMLRADIO_STATUS_STARTED 2

UiElements::UiElements(QObject *parent) :
    QObject(parent)
{
    QDBusConnection::sessionBus().connect("com.nokia.profiled",
                                          "/com/nokia/profiled",
                                          "com.nokia.profiled",
                                          "profile_changed", this, SLOT(onProfileChanged(const QDBusMessage &)));

    QDBusConnection::systemBus().connect("com.nokia.phone.SSC",
                                          "/com/nokia/phone/SSC",
                                          "com.nokia.phone.SSC",
                                          "pin_query_active", this, SLOT(onPinQueryActiveCHanged(bool)));


    QDBusConnection::systemBus().connect("com.nokia.phone.SSC",
                                          "/com/nokia/phone/SSC",
                                          "com.nokia.phone.SSC",
                                          "modem_state_changed_ind", this, SLOT(onCellularRadioChanged(const QString &)));

    QDBusConnection::systemBus().connect("com.nokia.phone.net",
                                          "/com/nokia/phone/net",
                                          "Phone.Net",
                                          "signal_strength_change", this, SLOT(onCellularSignalChanged(const uchar &, const uchar &)));
    QDBusConnection::systemBus().connect("com.nokia.phone.net",
                                          "/com/nokia/phone/net",
                                          "Phone.Net",
                                          "radio_access_technology_change", this, SLOT(onCellularRatChanged(const uchar &)));


    QDBusConnection::sessionBus().connect("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                                          "/com/nokia/mafw/renderer/gstrenderer",
                                          "com.nokia.mafw.renderer",
                                          "metadata_changed", this, SLOT(onMetaDataChanged(const QDBusMessage &)));
    QDBusConnection::sessionBus().connect("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                                          "/com/nokia/mafw/renderer/gstrenderer",
                                          "com.nokia.mafw.renderer",
                                          "state_changed", this, SLOT(onStateChanged(const int &)));

    QDBusConnection::systemBus().connect(QString(),
                                         "/org/freedesktop/Hal/devices/platform_headphone",
                                         "org.freedesktop.Hal.Device",
                                         "Condition", this, SLOT(onJackChanged(const QString &, const QString &)));

    QDBusConnection::sessionBus().connect(QString(),
                                          "/rtcomeventlogger/signal",
                                          "rtcomeventlogger.signal",
                                          "NewEvent", this, SLOT(onRtcomeventloggerEvent(const QDBusMessage &)));

    QDBusConnection::sessionBus().connect(QString(),
                                          "/org/qmlradio/signal",
                                          "org.qmlradio.signal",
                                          "status", this, SLOT(onQmlradioStatusChanged(const int &)));
    QDBusConnection::sessionBus().connect(QString(),
                                          "/org/qmlradio/signal",
                                          "org.qmlradio.signal",
                                          "freq", this, SLOT(onQmlradioFreqChanged(const int &)));

    tick = new QDBusInterface("com.nokia.mce",
                              "/com/nokia/mce/request",
                              "com.nokia.mce.request", QDBusConnection::systemBus());

    isLocked = false;
    wasPaused = false;
    checkNotification = true;
    isOnScreen = true;
    tagsEncoding = "Windows-1250";

    QSettings settings(QSettings::IniFormat,
                       QSettings::UserScope,
                       QCoreApplication::instance()->organizationName(),
                       "settings");

    if(settings.contains("tagsEncoding"))
        tagsEncoding = settings.value("tagsEncoding").toString();
    else
        settings.setValue("tagsEncoding", "Windows-1250");

    settings.sync();

    QTimer::singleShot(1000, this, SLOT(initEl()));
}

UiElements::~UiElements()
{
}

void UiElements::initEl()
{
    get_Profile();
    getCellularState();
    getBatteryPercentage();
    signalLevelChanged(getSignalLevel());

    mafw_status = getMafwStatus();
    if(mafw_status == MAFW_STATUS_STARTED)
        isPlay = true;
    else isPlay = false;

    emit mediaStatusChanged(isPlay);

    QDBusMessage msg = QDBusMessage::createMethodCall("org.qmlradio.control",
                                                      "/org/qmlradio/control",
                                                      "org.qmlradio.control",
                                                      "active");
    QDBusMessage qreply = QDBusConnection::sessionBus().call(msg);

    if(qreply.arguments().count() > 0)
        onQmlradioStatusChanged((qreply.arguments().at(0).toBool() ? 2 : 1));
    else
        onQmlradioStatusChanged(0);

    emit ratChanged(getRAT());

    QDBusInterface blueInterface("org.bluez",
                                 "/",
                                 "org.bluez.Manager", QDBusConnection::systemBus(), this);

    //http://maemo.org/api_refs/5.0/beta/bluez/index.html
    QDBusReply<QDBusObjectPath> blueReply = blueInterface.call("DefaultAdapter");

    if(blueReply.isValid())
    {
        QDBusObjectPath bluePath = blueReply.value();
        if(!bluePath.path().isEmpty())
        {
            QDBusConnection::systemBus().connect("org.bluez",
                                                 bluePath.path(),
                                                 "org.bluez.Adapter",
                                                 "PropertyChanged",
                                                 this,
                                                 SLOT(onBluetoothChanged(const QString &, const QDBusVariant &)));

            QDBusInterface powerInt("org.bluez",
                                    bluePath.path(),
                                    "org.bluez.Adapter",
                                    QDBusConnection::systemBus(), this);
            QDBusReply<QVariantMap> reply = powerInt.call("GetProperties");
            if(reply.isValid())
            {
                QVariantMap vmap = reply.value();
                emit blueToothChanged(vmap["Powered"].toBool());
            }
        }
    }
}

void UiElements::get_Profile()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.profiled",
                                   "/com/nokia/profiled",
                                   "com.nokia.profiled",
                                   "get_profile");
    QDBusReply<QString> reply = bus.call(msg);

    if(reply.isValid())
    {
        if(reply.value() == "general")
            isGeneral = true;
        else
            isGeneral = false;
        qDebug() << "get_profile" << reply.value();
        emit profileChanged(isGeneral);
    }
}

void UiElements::set_Silent()
{
    QDBusInterface prof("com.nokia.profiled",
                        "/com/nokia/profiled",
                        "com.nokia.profiled", QDBusConnection::sessionBus(), this);
    prof.asyncCall("set_profile", "silent");
}

void UiElements::set_General()
{
    QDBusInterface prof("com.nokia.profiled",
                        "/com/nokia/profiled",
                        "com.nokia.profiled", QDBusConnection::sessionBus(), this);
    prof.asyncCall("set_profile", "general");
}

void UiElements::toggleProfile()
{
    if(isGeneral)
        set_Silent();
    else
        set_General();
}

void UiElements::onProfileChanged(const QDBusMessage &msg)
{
    if(msg.arguments().at(2).toString() == "general")
    {
        emit profileChanged(true);
        isGeneral = true;
    }
    else if(msg.arguments().at(2).toString() == "silent")
    {
        emit profileChanged(false);
        isGeneral = false;
    }
}

void UiElements::openPhoneApp()
{
    runApp("rtcom-call-ui");
}

void UiElements::enableCellularRadio()
{
    QDBusInterface cellular("com.nokia.phone.SSC",
                            "/com/nokia/phone/SSC",
                            "com.nokia.phone.SSC", QDBusConnection::systemBus(), this);
    //cellular.call(QDBus::NoBlock, "set_radio", true);
    QDBusMessage reply = cellular.call("set_radio", true);
    qDebug() << "reply args: " << reply.arguments();
}

void UiElements::disableCellularRadio()
{
    QDBusInterface cellular("com.nokia.phone.SSC",
                            "/com/nokia/phone/SSC",
                            "com.nokia.phone.SSC", QDBusConnection::systemBus(), this);
    cellular.call(QDBus::NoBlock, "set_radio", false);
}

void UiElements::toggleCellularRadio()
{
    if(isOnline)
        disableCellularRadio();
    else
        enableCellularRadio();
}

void UiElements::onCellularRadioChanged(const QString &name)
{
    qDebug() << "onCellularRadioChanged:" << name;
    if(name == "online")
    {
        emit cellularRadioChanged(name);
        isOnline = true;
    }
    else if (name == "activation_pending")
    {
        emit cellularRadioChanged(name);
        isOnline = true;
    }
    else if (name == "inactive" || name == "tablet_state")
    {
        emit cellularRadioChanged(name);
        isOnline = false;
    }
}

void UiElements::getCellularState()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.phone.SSC",
                                   "/com/nokia/phone/SSC",
                                   "com.nokia.phone.SSC",
                                   "get_modem_state");
    QDBusReply<QString> reply = bus.call(msg);

    if(reply.isValid())
    {
        if(reply.value() == "online")
        {
            isOnline = true;
            emit cellularRadioChanged("online");
        }
        else if(reply.value() == "activation_pending")
        {
            isOnline = true;
            emit cellularRadioChanged("online");
        }
        else if(reply.value() == "inactive")
        {
            isOnline = false;
            emit cellularRadioChanged("inactive");
        }
        else if(reply.value() == "tablet_state")
        {
            isOnline = false;
            emit cellularRadioChanged("tablet_state");
        }
        qDebug() << "get_modem_state" << reply.value();
    }
}

void UiElements::sendUnLock()
{
    QDBusInterface unlock("com.nokia.mce", "/com/nokia/mce/request",
                          "com.nokia.mce.request", QDBusConnection::systemBus(), this);
    unlock.call(QDBus::NoBlock, "req_tklock_mode_change", "unlocked");
}

int UiElements::actualBrightness()
{
    QFile file("/sys/class/backlight/acx565akm/brightness");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return 0;
    QTextStream tx(&file);
    int br = tx.readLine().toInt();
    file.close();
    return br;
}

void UiElements::setBrightness(QVariant brightness)
{
    QDBusInterface brig("org.freedesktop.Hal",
                        "/org/freedesktop/Hal/devices/computer_backlight",
                        "org.freedesktop.Hal.Device.LaptopPanel", QDBusConnection::systemBus(), this);
    brig.call(QDBus::NoBlock, "SetBrightness", brightness.toInt());
}

void UiElements::disableBrightness()
{
    VariantAnimator *animator = new VariantAnimator();
    animator->setStartValue(actualBrightness());
    animator->setEndValue(0);
    animator->setDuration(400);
    connect(animator, SIGNAL(valueChanged(const QVariant&)), this, SLOT(setBrightness(QVariant)));
    connect(animator, SIGNAL(finished()), this, SLOT(brightnessFinished()));
    animator->start(QAbstractAnimation::DeleteWhenStopped);
}


void UiElements::brightnessFinished()
{
    QDBusInterface lock("com.nokia.mce", "/com/nokia/mce/request",
                          "com.nokia.mce.request", QDBusConnection::systemBus(), this);
    lock.call("req_tklock_mode_change", "locked");
}

void UiElements::getBatteryPercentage()
{
    int value = bq2700.getBatteryLevel();
    qDebug() << "i2c battery value: " << value;

    if(value > 100)
    {
        qDebug() << "\n\nwrong value: " << value;
        value = bq2700.getBatteryLevel();
        qDebug() << "value: " << value;
    }

    if(value != -1)
        emit batteryChanged(value);
}

void UiElements::onBatteryProcFinished(int exitCode, QProcess::ExitStatus status)
{
    if(status == QProcess::NormalExit)
    {
        QString output = QString(batteryProc->readLine());
        if(output != NULL)
        {
            output = output.split(" ").takeAt(4);
            emit batteryChanged(output.toInt());
            qDebug() << output;
        }
    }
    delete batteryProc;
    batteryProc = NULL;
}

void UiElements::onBluetoothChanged(const QString &name, const QDBusVariant &value)
{
    if(name == "Powered")
    {
        emit blueToothChanged(value.variant().toBool());
    }
}

void UiElements::onRealShowSignal()
{
    emit realShowSignal();
    QTimer::singleShot(0, this, SLOT(getBatteryPercentage()));
    signalLevelChanged(getSignalLevel());

}

uint UiElements::getPlaylistNumber()
{
      QDBusInterface pl("com.nokia.mafw.playlist",
                        "/com/nokia/mafw/playlist",
                        "com.nokia.mafw.playlist", QDBusConnection::sessionBus(), this);

      QDBusMessage msg2 = pl.call("create_playlist", "FmpAudioPlaylist");
      qDebug() << "interface getPlaylistNumber(): " << msg2.arguments().at(0).toUInt();

      return msg2.arguments().at(0).toUInt();
}

void UiElements::assignPlaylist()
{
    QDBusInterface pl("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                      "/com/nokia/mafw/renderer/gstrenderer",
                      "com.nokia.mafw.renderer", QDBusConnection::sessionBus(), this);

    pl.call("assign_playlist", getPlaylistNumber());
}

void UiElements::activateHildon()
{
    QDBusMessage _desktop = QDBusMessage::createSignal("/com/nokia/hildon_desktop",
                                                       "com.nokia.hildon_desktop",
                                                       "set_state");
    _desktop << 1;
    QDBusConnection::sessionBus().send(_desktop);
}

void UiElements::onShowSignal()
{
    emit showSignal();
}

void UiElements::setMAFWState(const QString &name)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                                                  "/com/nokia/mafw/renderer/gstrenderer",
                                                  "com.nokia.mafw.renderer", name);
    QDBusConnection::sessionBus().asyncCall(msg);
}

void UiElements::setQmlradioState(const QString &name)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.qmlradio.control",
                                                      "/org/qmlradio/control",
                                                      "org.qmlradio.control",
                                                      name);
    QDBusConnection::sessionBus().asyncCall(msg);
}
void UiElements::setQmlradioState(const QString &name, const bool &arg)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.qmlradio.control",
                                                      "/org/qmlradio/control",
                                                      "org.qmlradio.control",
                                                      name);
    msg << arg;
    QDBusConnection::sessionBus().asyncCall(msg);
}

void UiElements::play()
{

    if(qmlradio_status == QMLRADIO_STATUS_STARTED ||
       qmlradio_status == QMLRADIO_STATUS_STOPED)
    {
        setQmlradioState("setActive", ((qmlradio_status == QMLRADIO_STATUS_STARTED) ? false : true));
    }
    else {
        QString name;

        qDebug() << "play() - mafwStatus:" << mafw_status;

        if(mafw_status == MAFW_STATUS_STARTED)
            name = "pause";
        else if(mafw_status == MAFW_STATUS_PAUSED)
            name = "resume";
        else if (mafw_status == MAFW_STATUS_STOPPED)
        {
            name = "play";
            if(!getLocalTagFs().startsWith("localtagfs::music/"))
                assignPlaylist();
        }
        else return;

        setMAFWState(name);
    }
}

void UiElements::stop()
{
    if(qmlradio_status == QMLRADIO_STATUS_STARTED)
    {
        setQmlradioState("setActive", false);
    }
    else {
        setMAFWState("stop");
    }
}

void UiElements::previous()
{
    if(qmlradio_status == QMLRADIO_STATUS_STARTED ||
       qmlradio_status == QMLRADIO_STATUS_STOPED)
    {
        setQmlradioState("prev");
    }
    else {
        setMAFWState("previous");
    }
}

void UiElements::next()
{
    if(qmlradio_status == QMLRADIO_STATUS_STARTED ||
       qmlradio_status == QMLRADIO_STATUS_STOPED)
    {
        setQmlradioState("next");
    }
    else {
        setMAFWState("next");
    }
}

QString UiElements::getCurrentDate()
{
    return QDate::currentDate().toString("dddd, dd/MM/yyyy");
}

//dbus-send --print-reply --type=method_call --dest=com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer /com/nokia/mafw/renderer/gstrenderer com.nokia.mafw.extension.list_extension_properties
//dbus-send --print-reply --type=method_call --dest=com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer /com/nokia/mafw/renderer/gstrenderer com.nokia.mafw.renderer.get_current_metadata
//dbus-send --print-reply --type=method_call --dest=com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer /com/nokia/mafw/renderer/gstrenderer com.nokia.mafw.extension.get_extension_property string:volume
void UiElements::onMetaDataChanged(const QDBusMessage &msg)
{
    QList<QVariant> values = msg.arguments();
    if(values.at(0).toString() == "title" )
    {
        QDBusVariant var = qvariant_cast<QDBusVariant>(values.at(2));
        QTextCodec *codec = QTextCodec::codecForName(tagsEncoding.toLatin1());
        emit songTitleChanged(codec->toUnicode(var.variant().toByteArray()));
    }
    else if(values.at(0).toString() == "artist" )
    {
        QDBusVariant var = qvariant_cast<QDBusVariant>(values.at(2));
        QTextCodec *codec = QTextCodec::codecForName(tagsEncoding.toLatin1());
        emit songArtistChanged(codec->toUnicode(var.variant().toByteArray()));
    }
}

void UiElements::onCellularSignalChanged(const uchar &bar, const uchar &rssi)
{
    if(isOnScreen)
       emit signalLevelChanged(bar);
}

void UiElements::onCellularRatChanged(const uchar &rat)
{
    emit ratChanged(rat);
}

void UiElements::onStateChanged(const int &state)
{
    mafw_status = state;

    if(mafw_status == MAFW_STATUS_STOPPED)
    {
        isPlay = false;
        emit mediaStatusChanged(isPlay);
    }
    else if(mafw_status == MAFW_STATUS_STARTED)
    {
        isPlay = true;
        wasPaused = false;
        emit mediaStatusChanged(isPlay);
    }
    else if(mafw_status == MAFW_STATUS_PAUSED)
    {
        isPlay = false;
        emit mediaStatusChanged(isPlay);
    }

}

int UiElements::getMafwStatus()
{
    //http://alterego.metapath.org/projects/shared/mafw_support.py
    QDBusInterface st("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                      "/com/nokia/mafw/renderer/gstrenderer",
                      "com.nokia.mafw.renderer", QDBusConnection::sessionBus(), this);

    QDBusMessage msg2 = st.call("get_status");

    if(msg2.arguments().count() == 4)
    {
            int reply = msg2.arguments().at(2).toInt();

            mafw_status = reply;
            qDebug() << "getMafwStatus(): " << reply;

            return mafw_status;
    }
    else return int();
}

QString UiElements::getLocalTagFs()
{
    QDBusInterface st("com.nokia.mafw.renderer.Mafw-Gst-Renderer-Plugin.gstrenderer",
                      "/com/nokia/mafw/renderer/gstrenderer",
                      "com.nokia.mafw.renderer", QDBusConnection::sessionBus(), this);

    QDBusMessage msg = st.call("get_status");

    if(msg.arguments().count() == 4)
    {
        return msg.arguments().at(3).toString();
    }
    else return QString();
}

int UiElements::getSignalLevel()
{
    QDBusMessage msg = QDBusMessage::createMethodCall("com.nokia.phone.net",
                                                      "/com/nokia/phone/net",
                                                      "Phone.Net",
                                                      "get_signal_strength");

    QDBusMessage reply = QDBusConnection::systemBus().call(msg);
    if(reply.arguments().count() > 1)
    {
        return reply.arguments().at(0).toInt();
    }
    return 0;
}

//radio access technology
//https://qt.gitorious.org/qt-mobility/fenghaitaos-simulator-mobility/source/ad55e9b31cbd44ac851b75488ce04f361362ce85:src/systeminfo/qsysteminfo_maemo.cpp#L564-599
int UiElements::getRAT()
{
    QDBusInterface connInterface("com.nokia.phone.net",
                                 "/com/nokia/phone/net",
                                 "Phone.Net",
                                  QDBusConnection::systemBus());


    QDBusMessage reply = connInterface.call("get_radio_access_technology");
    if(reply.arguments().count() > 1)
    {
        return reply.arguments().at(0).toInt();
    }
    return 0;
}

void UiElements::onOffScreen()
{
    emit offSignal();

    isOnScreen = false;

    if(checkNotification)
    {
        bool isCall = false;
        bool isSms = false;
            QList<Window> wlist = X11Utils::getWindowList();
            for (int i = 0; i < wlist.count(); ++i)
            {
                if(X11Utils::getWindowType(wlist.at(i)) == X11Utils::returnAtom("_NET_WM_WINDOW_TYPE_NOTIFICATION"))
                {
                    QString wtype = X11Utils::getWindowNotificationType(wlist.at(i));

                    if(wtype == "Rtcom-call-ui")
                    {
                        qDebug() << "Rtcom-call-ui";
                        isCall = true;
                    }
                    else if(wtype == "Rtcom-messaging-ui")
                    {
                        qDebug() << "Rtcom-messaging-ui";
                        isSms = true;
                    }
                    else {
                        qDebug() << "WindowNotificationType: " << wtype;
                    }
                }
            }
            if(!isCall)
                emit callEvent("clear");
            if(!isSms)
                emit smsEvent("clear");
            if(isCall || isSms)
                checkNotification = true;
            else
                checkNotification = false;
    }
}

void UiElements::onOffLockedScreen()
{
    isOnScreen = false;
}

void UiElements::onOnLockedScreen()
{
    isOnScreen = true;
    checkProximityCover();
}


void UiElements::checkProximityCover()
{
    if(isOnScreen) {
        if(proximityCovered())
        {
            emit setMouseEvents(false);
            QTimer::singleShot(400, this, SLOT(checkProximityCover()));
        }
        else
        {
            emit setMouseEvents(true);
            QTimer::singleShot(400, this, SLOT(checkProximityCover()));

        }
    }
}

void UiElements::runApp(QString name)
{
    sendUnLock();
    QDBusInterface ph("com.nokia.HildonDesktop.AppMgr",
                      "/com/nokia/HildonDesktop/AppMgr",
                      "com.nokia.HildonDesktop.AppMgr", QDBusConnection::sessionBus(), this);
    ph.call(QDBus::NoBlock, "LaunchApplication", name);
}

void UiElements::sendLock()
{
    QDBusInterface unlock("com.nokia.mce", "/com/nokia/mce/request",
                          "com.nokia.mce.request", QDBusConnection::systemBus(), this);
    unlock.call("req_tklock_mode_change", "locked");
}

void UiElements::keepAliveScreen()
{
    QDBusInterface tick("com.nokia.mce",
                        "/com/nokia/mce/request",
                        "com.nokia.mce.request", QDBusConnection::systemBus(), this);
    tick.asyncCall("req_display_blanking_pause");    
}


void UiElements::setVibration(int value)
{
    QFile file("/sys/class/leds/twl4030:vibrator/dbrightness");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream st(&file);
        st << value;
        file.close();
    }
}

void UiElements::tickVibrate()
{
    tick->asyncCall("req_vibrator_pattern_activate", "PatternTouchscreen");
}

void UiElements::onJackChanged(const QString &msg1, const QString &msg2)
{
    if(msg1 == "ButtonPressed" || msg2 == "connection")
    {
        QFile file("/sys/devices/platform/gpio-switch/headphone/state");
        if(file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream st(&file);
            if(st.readLine() == "disconnected")
            {
                if(mafw_status == MAFW_STATUS_STARTED)
                {
                    setMAFWState("pause");
                    wasPaused = true;
                }
            }
            else // "connected"
            {
                 if(mafw_status == MAFW_STATUS_PAUSED && isLocked && wasPaused)
                 {
                     play(); //resume
                 }
                 wasPaused = false;
            }
        file.close();
        }
    }
}


bool UiElements::proximityCovered()
{
    QFile file("/sys/devices/platform/gpio-switch/proximity/state");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream tx(&file);
        QString st = tx.readLine();
        file.close();
        if(st == "closed")
           return true;
    }
    return false;
}

void UiElements::onQmlradioStatusChanged(const int &status)
{
    qmlradio_status = status;

    if(status == QMLRADIO_STATUS_STARTED)
    {
        emit mediaStatusChanged(true);

        QDBusMessage msg = QDBusMessage::createMethodCall("org.qmlradio.control",
                                                          "/org/qmlradio/control",
                                                          "org.qmlradio.control",
                                                          "freq");
        QDBusMessage qreply = QDBusConnection::sessionBus().call(msg);

        if(qreply.arguments().count() == 1)
            onQmlradioFreqChanged(qreply.arguments().at(0).toInt());
    }
    else if(status == QMLRADIO_STATUS_STOPED)
    {
        emit songTitleChanged("Qmlradio");
        emit songArtistChanged("");
        setMAFWState("pause");
        emit mediaStatusChanged(false);
    }
    else if(status == QMLRADIO_STATUS_NULL)
    {
        emit songTitleChanged("Song Title");
        emit songArtistChanged("Artist");
        emit mediaStatusChanged(false);
    }
}

void UiElements::onQmlradioFreqChanged(const int &freq)
{
    emit songArtistChanged(QString("%1.%2Mhz").arg(
                           QString::number(freq/1000),
                           QString::number(freq%1000)));
}

void UiElements::onRtcomeventloggerEvent(const QDBusMessage &msg)
{
    if(msg.arguments().at(5) == "RTCOM_EL_SERVICE_CALL")
    {
        qDebug() << "call event";
        checkNotification = true;
        emit callEvent("add");
    }
    else if(msg.arguments().at(5) == "RTCOM_EL_SERVICE_SMS")
    {
        checkNotification = true;
        qDebug() << "sms event";
        emit smsEvent("add");
    }
    else {
        checkNotification = false;
    }
}

void UiElements::onPinQueryActiveCHanged(bool active)
{
    qDebug() << "onPinQueryActiveCHanged";
    if(active)
    {
        sendUnLock(); //NEED test
    }
}
