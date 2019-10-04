import QtQuick 1.1

Item {
    property alias textColor: txt.color
    property alias text: txt.text
    Text {
        id: txt
        color: "grey"
        anchors {
            centerIn: parent
        }
        font.pixelSize: 30
        font.weight: Font.Light
        font.family: mainFont.name
    }
}
