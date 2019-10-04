import QtQuick 1.1

Rectangle {
    id: rec1
    signal iconPressed()
    width: parent.width
    height: 100
    color: "black"
    Row {
        id: iconsRow
        anchors.centerIn: parent
        width: (64 + spacing)*children.length - spacing
        height: 64
        spacing: isPortrait ? 30 : 50

    }
    Component.onCompleted: {
        addItems()
    }

    function addItems()
    {
        Settings.beginGroup("QuickBar")

        var icons = Settings.allKeys()

        if(icons == "") //load default
        {
            Settings.setValue(
                        "rtcom-messaging-ui",
                        "file://usr/share/icons/hicolor/64x64/hildon/tasklaunch_sms_chat.png")
            Settings.setValue(
                        "osso-addressbook",
                        "file://usr/share/icons/hicolor/64x64/hildon/general_contacts.png")
            Settings.setValue(
                        "image-viewer",
                        "file://usr/share/icons/hicolor/64x64/hildon/tasklaunch_photos.png")
            Settings.setValue(
                        "osso-xterm",
                        "file://usr/share/icons/hicolor/64x64/apps/terminal.png")

            icons = Settings.allKeys()
        }

        for (var i = 0; i < Settings.allKeys().length; ++i)
        {
            var component = Qt.createComponent("Icon.qml")
            var icon = component.createObject(iconsRow)
            icon.source = Settings.value(icons[i])
            icon.exec = icons[i]
        }

        Settings.endGroup()
    }
}
