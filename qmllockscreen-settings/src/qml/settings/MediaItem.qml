import QtQuick 1.1

Item {
    id: mediaI
    width: 378
    height: 150

    Rectangle {
        width: parent.width - 1
        height: parent.height - 1
        border.color: "#54af60"
        color: "transparent"
        radius: 4
        opacity: 0.4
    }

    Item {
        width: parent.width - 2
        height: parent.height - 2
        anchors.centerIn: parent
        clip: true
        Rectangle {
            anchors.fill: parent
            color: "white"
            radius: 4
            opacity: 0.05
        }


        Rectangle {
            z: 10
            anchors {
                centerIn: parent
            }
            width: parent.width
            height: parent.height
            color: "black"
            radius: 4
            opacity: leftRec.opacity || rightRec.opacity || topRec.opacity || bottomRec.opacity
        }
        Item {
            id: mi
            width: parent.width
            height: parent.height


            MouseArea {
                id: mia
                anchors.fill: parent
                drag.target: mi
                property int mw: parent.width / 2
                property int mh: parent.height / 2
                drag.maximumX: mw
                drag.minimumX: -mw
                drag.maximumY: mh
                drag.minimumY: -mh
                property int counter: 0
                property bool tick: false
            }
        }

        Item {
            id: leftRec
            z: 10
            property int pwidth2 : parent.width / 2
            anchors {
                right: mi.left
                rightMargin: -pwidth2
            }
            width: parent.width
            height: parent.height
            opacity: (mia.counter <= 11) ? 0 : (leftRec.x + pwidth2) / parent.width
            Image {
                asynchronous: true
                anchors {
                    centerIn: parent
                }
                source: "file://" + appPath + "/icons/previous.png"
            }

        }
        Item {
            id: rightRec
            z: 10
            property int pwidth2 : parent.width / 2
            anchors {
                left: mi.right
                leftMargin: -pwidth2
            }
            width: parent.width
            height: parent.height
            opacity: (mia.counter <= 11) ? 0 : (pwidth2 - rightRec.x) / parent.width
            Image {
                asynchronous: true
                anchors {
                    centerIn: parent
                }
                source: "file://" + appPath + "/icons/next.png"
            }
        }
        Item {
            id: topRec
            z: 10
            property int pheight2 : parent.height / 2
            anchors {
                bottom: mi.top
                bottomMargin: -pheight2
            }
            width: parent.width
            height: parent.height
            opacity: (mia.counter <= 11) ? 0 : (pheight2 + topRec.y) / parent.height
            Image {
                id: timage
                asynchronous: true
                anchors {
                    centerIn: parent
                }
                source: "file://" + appPath + "/icons/play.png"
            }
        }
        Item {
            id: bottomRec
            z: 10
            property int pheight2 : parent.height / 2
            anchors {
                top: mi.bottom
                topMargin: -pheight2
            }
            width: parent.width
            height: parent.height
            opacity: (mia.counter <= 11) ? 0 : (pheight2 - bottomRec.y) / parent.height
            Image {
                id: bimage
                asynchronous: true
                anchors {
                    centerIn: parent
                }
                source: "file://" + appPath + "/icons/stop.png"
            }

        }



        Text {
            z: 5
            id: titleText
            text: "Song Title"
            font.pixelSize: 30
            font.family: mainFont.name
            font.weight: Font.Light
            color: "white"
            anchors {
                left: parent.left
                leftMargin: 20
                top: parent.top
                topMargin: 30
            }
        }
        Text {
            id: artistText
            z: 5
            text: "Artist"
            font.pixelSize: 20
            font.family: mainFont.name
            font.weight: Font.Light
            color: "white"
            anchors {
                left: parent.left
                leftMargin: 20
                top: titleText.bottom
                topMargin: 10
            }
        }

    }


}
