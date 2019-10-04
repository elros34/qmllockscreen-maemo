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
            Behavior on x {
                id: xBehavior
                NumberAnimation {
                    id: xAnim
                    onRunningChanged: {
                        if (!xAnim.running) {

                        }
                    }
                }
            }
            Behavior on y {
                id: yBehavior
                NumberAnimation {
                    id: yAnim
                    onRunningChanged: {
                        if (!yAnim.running) {

                        }
                    }
                }
            }

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
                onPressed: {
                    counter = 0
                    xBehavior.enabled = false
                    yBehavior.enabled = false
                    tick = false
                }
                onPositionChanged: {
                    ++counter
                    if (counter == 11) {
                        if (Math.abs(mi.x) > Math.abs(mi.y)) {
                            drag.maximumY = 0
                            drag.minimumY = 0
                        } else {
                            drag.maximumX = 0
                            drag.minimumX = 0
                        }
                    }
                }

                onReleased: {
                    if (mi.x == mw)
                        uiTools.previous()
                    else if (mi.x == -mw)
                        uiTools.next()
                    else if (mi.y == mh)
                        uiTools.play()
                    else if(mi.y == -mh)
                        uiTools.stop()

                    xBehavior.enabled = true
                    yBehavior.enabled = true
                    mi.x = 0
                    mi.y = 0
                    drag.maximumX = mw
                    drag.minimumX = -mw
                    drag.maximumY = mh
                    drag.minimumY = -mh
                }
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

            onOpacityChanged: {
                if(opacity == 0.5)
                    uiTools.tickVibrate()
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
            onOpacityChanged: {
                if(opacity == 0.5)
                    uiTools.tickVibrate()
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
            onOpacityChanged: {
                if(opacity == 0.5)
                    uiTools.tickVibrate()
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
            onOpacityChanged: {
                if(opacity == 0.5)
                    uiTools.tickVibrate()
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



    Connections {
        target: uiTools
        onSongTitleChanged: {
            titleText.text = stitle
        }
        onSongArtistChanged: {
            artistText.text = sartist
        }
        onMediaStatusChanged: {
            is_play = isPlay
            playTimer.start()
        }
    }

    property bool is_play
    Timer {
        id: playTimer
        running: false
        interval: 250
        onTriggered: {
            if(is_play)
            {
                timage.source = "file://" + appPath + "/icons/pause.png"
            }
            else {
                timage.source = "file://" + appPath + "/icons/play.png"
            }
        }
    }
}
