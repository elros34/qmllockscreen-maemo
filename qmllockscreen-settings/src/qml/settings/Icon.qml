import QtQuick 1.1

Image {
    property string exec
    MouseArea {
        anchors.fill: parent
        onClicked: {
            rec1.iconPressed()

            fileSelect.visible = true
            fileSelect.fileSelected.connect(changeShortcut)
            fileModel.setFilters("*.desktop")
            fileModel.setDir("/usr/share/applications/")
            mainWindow.z = 30
        }
    }
    function changeShortcut(path) {
        console.log(path)

        if(path) {
            var desktopItem = fileModel.getDesktopFileInfo(path)
            Settings.beginGroup("QuickBar")
            Settings.remove(exec)
            Settings.setValue(desktopItem[0], desktopItem[1])
            Settings.endGroup()

            exec = desktopItem[0]
            source = "file://"+desktopItem[1]
        }


        fileSelect.visible = false
        fileSelect.fileSelected.disconnect(changeShortcut)
        mainWindow.z = 10
    }

}
