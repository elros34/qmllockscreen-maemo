#ifndef UIELEMENTS_H
#define UIELEMENTS_H

#include <QObject>
#include <QtDBus>
#include <QFile>
#include <QProcess>
#include "VariantAnimator.h"
#include <X11/Xlib.h>
#include "x11utils.h"
#include "bq2700.h"

class UiElements : public QObject
{
    Q_OBJECT
public:
    explicit UiElements(QObject *parent = 0);

    ~UiElements();

    bool isLocked;

private:
    bool isGeneral;
    bool isOnline;
    bool isPlay;
    bool isFirst;
    bool wasPaused;
    bool checkNotification;
    bool isOnScreen;
    int mafw_status;
    int qmlradio_status;
    QString tagsEncoding;

    QDBusInterface *tick;
    BQ2700 bq2700;

    QProcess *batteryProc;

signals:
    void profileChanged(bool is_general);
    void cellularRadioChanged(const QString &radioState);
    void batteryChanged(int bp);
    void realShowSignal();
    void showSignal();
    void songTitleChanged(QString stitle);
    void songArtistChanged(QString sartist);
    void mediaStatusChanged(bool isPlay);
    void offSignal();
    void callEvent(QString msg);
    void smsEvent(QString msg);
    void setMouseEvents(bool enable);
    void blueToothChanged(bool enabled);
    void signalLevelChanged(int level);
    void ratChanged(const int &value);

public slots:
    void toggleProfile();
    void openPhoneApp();
    void toggleCellularRadio();
    void sendUnLock();
    void setBrightness(QVariant brightness);
    void disableBrightness();
    void onRealShowSignal();
    void onOffScreen();
    void onOffLockedScreen();
    void onOnLockedScreen();
    void activateHildon();
    void onShowSignal();
    void runApp(QString name);
    void sendLock();
    void keepAliveScreen();
    void tickVibrate();

    void previous();
    void next();
    void stop();

    bool proximityCovered();

    void getBatteryPercentage();
    void play();
    QString getCurrentDate();

private slots:
    void initEl();
    void get_Profile();
    void set_Silent();
    void set_General();
    void getCellularState();
    void enableCellularRadio();
    void disableCellularRadio();
    void onProfileChanged(const QDBusMessage &msg);
    void onCellularRadioChanged(const QString &name);
    void onPinQueryActiveCHanged(bool active);
    void onMetaDataChanged(const QDBusMessage &msg);
    void onCellularSignalChanged(const uchar &bar, const uchar &rssi);
    void onCellularRatChanged(const uchar &rat);
    void onStateChanged(const int &state);
    void assignPlaylist();
    void brightnessFinished();
    void onJackChanged(const QString &msg1, const QString &msg2);
    void onRtcomeventloggerEvent(const QDBusMessage &msg);
    void checkProximityCover();
    void onQmlradioStatusChanged(const int &status);
    void onQmlradioFreqChanged(const int &freq);

    int actualBrightness();
    uint getPlaylistNumber();
    int getMafwStatus();
    QString getLocalTagFs();
    int getSignalLevel();
    int getRAT();

    void setVibration(int value = 0);
    void onBatteryProcFinished(int exitCode, QProcess::ExitStatus status);
    void onBluetoothChanged(const QString &name, const QDBusVariant &value);
    void setMAFWState(const QString &name);
    void setQmlradioState(const QString &name);
    void setQmlradioState(const QString &name, const bool &arg);
    
};

#endif // UIELEMENTS_H
