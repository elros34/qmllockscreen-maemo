import QtQuick 1.1

Item {
    width: isPortrait ? 480 : 800
    height: 56

    MouseArea {
        anchors.fill: parent
    }

    Rectangle {
        anchors.fill: parent
        opacity: 0.5
        color: "black"
    }

    Item {
        x: 0
        id: task
        height: parent.height
        width: isPortrait  ? 80 : 112

        Image {
            id: taskIcon
            source: "file://etc/hildon/theme/images/wmTaskSwitcherIcon.png"
            anchors.centerIn: parent

            MouseArea {
                id: taskArea
                anchors.fill: parent
                onClicked: mainView.minimize();
            }

            states: State {
                name: "pressed"
                when: taskArea.pressed
                PropertyChanges {
                    target: taskIconPressed
                    visible: true
                }
            }
        }
        Image {
            id: taskIconPressed
            source: "file://etc/hildon/theme/images/wmTaskSwitcherIconPressed.png"
            visible: false
            anchors.centerIn: parent
        }
        Image {
            id: separator
            anchors{
                top: parent.top; bottom: parent.bottom; right: parent.right
            }
            source: "file://etc/hildon/theme/images/wmSeparator.png"
        }
    }

    Image {
        id: statusAreaImage
        anchors.left: task.right
        cache: false
        property int wid:  0

        Connections {
            target: mainView
            onUpdateStatusArea: {
                statusAreaImage.source = ""
                statusAreaImage.source = "image://imageProvider/"+ wId
            }
        }
    }


    Image {
        id: separator1
        anchors{
            top: parent.top; bottom: parent.bottom; left: statusAreaImage.right
        }
        source: "file://etc/hildon/theme/images/wmSeparator.png"
    }

    Text {
        anchors {
            verticalCenter: parent.verticalCenter
            left: separator1.right
            leftMargin: 20
            right: close.left
        }
        verticalAlignment: Text.AlignVCenter
        height: 40
        color: "white"
        wrapMode: Text.WrapAnywhere
        clip: true
        font.family: lightFont.name
        text: "Qmllockscreen-settings"
    }

    Item {
        id: close
        height: parent.height
        anchors.right: parent.right

        width: isPortrait  ? 80 : 112
        Image {
            id: closeButton
            z: 11
            source: "file://etc/hildon/theme/images/wmCloseIcon.png"
            anchors.centerIn: parent
            MouseArea {
                id: closeArea
                anchors.fill: parent
                onClicked: mainView.close()
            }

        }
        Image {
            id: separator2
            anchors{
                top: parent.top; bottom: parent.bottom; left: parent.left
            }
            source: "file://etc/hildon/theme/images/wmSeparator.png"
        }

    }


}
