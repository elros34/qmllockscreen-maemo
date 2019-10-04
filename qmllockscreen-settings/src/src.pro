#-------------------------------------------------
#
# Project created by QtCreator 2013-10-21T15:36:29
#
#-------------------------------------------------

QT       += core gui dbus declarative
LIBS += -lX11 -lXcomposite -lXdamage

TARGET = qmllockscreen-settings

TEMPLATE = app

SOURCES += \
    main.cpp \
    core.cpp \
    mainview.cpp \
    imageprovider.cpp \
    x11helper.cpp \
    appitem.cpp \
    filemodel.cpp

HEADERS += \
    core.h \
    mainview.h \
    imageprovider.h \
    x11helper.h \
    appitem.h \
    Settings.h \
    filemodel.h

OBJECTS_DIR = .obj
MOC_DIR = .moc

OTHER_FILES += \
    qml/settings/main.qml \
    qml/settings/HildonBar.qml \
    qml/settings/Button.qml \
    qml/settings/FileItem.qml \
    qml/settings/LockView.qml \
    qml/settings/FileSelect.qml \
    qml/settings/MediaItem.qml \
    qml/settings/MainWindow.qml \
    qml/settings/Events.qml \
    qml/settings/QuickBar.qml \
    qml/settings/Icon.qml

maemo5 {
    target.path = /opt/qmllockscreen-settings/

    qml.path = /opt/qmllockscreen-settings/qml/settings
    qml.files += \
    qml/main.qml \

    INSTALLS += target  qml
}
