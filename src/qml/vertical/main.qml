import QtQuick 1.1
import Utils 1.0

Item {
    id: mainW
    width: isPortrait ? 480 : 800
    height: isPortrait ? 800 : 480
    property string wid
    property int fullScreenWId: 0

    Rectangle {
        id: windowBar
        visible: false
        height: 56
        color: "black"
        anchors {
            bottom: windowItem.top
            left: parent.left
            right: parent.right
        }
    }

    WindowItem {
        id: windowItem
        source: "grab"
        height: parent.height
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
    }

    LockView {
        id: lockView
        z: 100
        onShowQuickBar: {
            quickBar.visible = true
        }
        onHideQuickBar: {
            quickBar.visible = false
        }
    }


    QuickBar {
        id: quickBar
        z: 50
        visible: false
        anchors.bottom: parent.bottom
        onIconPressed: {
            lockView.y = 0
            lockView.isqmode = false
            visible = false
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

    Connections {
        target: mainLock
        onUpdateBottomView: {
            windowItem.source = wId + "/" + (isHildon ? isFWId : 1)
            wid = wId
            fullScreenWId = isFWId
            if (fullScreenWId) {
                windowItem.height = mainW.height
                windowBar.visible = false
            } else {
                windowItem.height = mainW.height - 56
                windowBar.visible = true
            }
        }
    }
}
