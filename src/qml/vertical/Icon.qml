import QtQuick 1.1

Image {
    property string exec
    MouseArea {
        anchors.fill: parent
        onClicked: {
            uiTools.runApp(exec)
            rec1.iconPressed()
        }
    }
}
