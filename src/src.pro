QT       += core dbus gui declarative
LIBS += -lX11 -lXcomposite -lXdamage
CONFIG += link_pkgconfig
PKGCONFIG += gconf-2.0 glib-2.0

TARGET = qmllockscreen

TEMPLATE = app

QMAKE_CXXFLAGS += -fPIC -fvisibility=hidden -fvisibility-inlines-hidden
QMAKE_LFLAGS += -pie -rdynamic

SOURCES += main.cpp \
    lscore.cpp \
    mainlock.cpp \
    windowitem.cpp \
    x11utils.cpp \
    hildonutils.cpp \
    uielements.cpp \
    bq2700.cpp


HEADERS += \
    lscore.h \
    mainlock.h \
    windowitem.h \
    x11utils.h \
    hildonutils.h \
    uielements.h \
    VariantAnimator.h \
    Settings.h \
    bq2700.h



OTHER_FILES += \
    qml/vertical/main.qml \
    qml/vertical/LockView.qml \
    qml/vertical/TopMenu.qml \
    qml/vertical/TopMenuItem.qml \
    qml/vertical/QuickBar.qml \
    qml/vertical/MediaItem.qml \
    qml/vertical/Icon.qml \
    qml/vertical/Events.qml \
    qml/vertical-v2/main.qml \
    qml/vertical-v2/LockView.qml \
    qml/vertical-v2/TopMenu.qml \
    qml/vertical-v2/TopMenuItem.qml \
    qml/vertical-v2/QuickBar.qml \
    qml/vertical-v2/MediaItem.qml \
    qml/vertical-v2/Icon.qml \
    qml/vertical-v2/Events.qml \
    qml/horizontal/main.qml \
    qml/horizontal/LockView.qml \
    qml/horizontal/TopMenu.qml \
    qml/horizontal/TopMenuItem.qml \
    qml/horizontal/QuickBar.qml \
    qml/horizontal/MediaItem.qml \
    qml/horizontal/Icon.qml \
    qml/horizontal/Events.qml

    OBJECTS_DIR = .obj
    MOC_DIR = .moc

    target.path = /opt/qmllockscreen/

    qml.path = /opt/qmllockscreen/qml/
    qml.files += qml/*

    font.path = /opt/qmllockscreen/
    font.files += font

    images.path = /opt/qmllockscreen/
    images.files += icons

#    systemui.path = /opt/qmllockscreen/
#    systemui.files += ../blank-tklock/libsystemuiplugin_tklock.so

    autostart.path = /opt/qmllockscreen/
    autostart.files += qmllockscreen.sh

    i2c.path = /opt/qmllockscreen/
    i2c.files += chmod_i2c.sh

    INSTALLS += target qml font images autostart i2c


