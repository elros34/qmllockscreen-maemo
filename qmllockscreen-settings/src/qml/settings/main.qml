import QtQuick 1.1

Item {
    width: isRealPortrait ? 480 : 800
    height: isRealPortrait ? 800 : 480

    Item {
        id: rotatorItem
        width: isPortrait ? 480 : 800
        height: isPortrait ? 800 : 480
        anchors.centerIn: parent
        rotation: {
            if (isRealPortrait == isPortrait)
                return 0
            else if (!isRealPortrait && isPortrait)
                return -90
            else if (isRealPortrait && !isPortrait)
                return 90
        }
        HildonBar {
            z: 20
            id: topBar
        }

        MainWindow {
            id: mainWindow
            z: 10

            Button {
                width: 70
                height: 60
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                }
                onClicked: topBar.visible = !topBar.visible
            }
        }

    }
    FontLoader {
        id: mainFont
        source: "file:/" + appPath + "/OpenSans-Regular.ttf"
    }
    FontLoader {
        id: lightFont
        source: "file:/" + appPath + "/OpenSans-Light.ttf"
    }
}
