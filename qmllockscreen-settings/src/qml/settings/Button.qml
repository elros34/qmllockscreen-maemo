import QtQuick 1.1

Rectangle {
    property alias text: txt.text
    signal clicked
    width: 100
    height: 62
    radius: 2
    border.color: "grey"
    smooth: true
    color: "black"
    opacity: 0.7

    Text {
        id: txt
        anchors.centerIn: parent
        color: "white"
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: parent.clicked()
        onPressed: parent.color = "#404244"
        onReleased: parent.color = "black"
    }
}
