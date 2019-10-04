import QtQuick 1.1

Rectangle {
    id: lockView
    width: isPortrait ? 480 : 800
    height: isPortrait ? 800 : 480

    property string wallpaperPath

    Image {
        rotation: isPortrait ? 0 : -90
        source: "file://" + wallpaperPath
        anchors.centerIn: parent

        MouseArea {
            anchors.fill: parent
            drag.target: lockView
            drag.axis: Drag.YAxis
            drag.maximumY: 0
            drag.minimumY: -quickBar.height
            onPressAndHold: {
                fileSelect.visible = true
                fileSelect.fileSelected.connect(changeWallpaper)
                fileModel.setFilters(["*.png","*.jpg","*.jpeg"])
                fileModel.setDir("/home")
                mainWindow.z = 30
            }
            onReleased: {
                if(lockView.y > -quickBar.height/2)
                    lockView.y = 0
                else
                    lockView.y = -quickBar.height
            }
        }
    }

    QuickBar {
        id: quickBar
        anchors.top: parent.bottom
    }

    function changeWallpaper(path) {
        if(path) {
            wallpaperPath = path
            Settings.beginGroup("lockscreen")
            Settings.setValue("wallpaperPath", path)
            Settings.endGroup()
        }
        fileSelect.fileSelected.disconnect(changeWallpaper)
        fileSelect.visible = false
        mainWindow.z = 10
    }

    MediaItem {
        z: 10
        id: mediaItem
        x: isPortrait ? mediaItemLeftMarginP : mediaItemLeftMarginL
        y: isPortrait ? mediaItemTopMarginP : mediaItemTopMarginL
        MouseArea {
            id: mediaItemArea
            anchors.fill: parent
            drag.target: mediaItem
            drag.axis: Drag.XandYAxis
            onReleased: {
                xline.visible = false
                if (isPortrait) {
                    mediaItemLeftMarginP = parent.x
                    mediaItemTopMarginP = parent.y
                    Settings.beginGroup("portrait")
                    Settings.setValue("mediaItemLeftMargin", mediaItemLeftMarginP)
                    Settings.setValue("mediaItemTopMargin", mediaItemTopMarginP)
                    Settings.endGroup()
                } else {
                    mediaItemLeftMarginL = parent.x
                    mediaItemTopMarginL = parent.y
                    Settings.beginGroup("landscape")
                    Settings.setValue("mediaItemLeftMargin", mediaItemLeftMarginL)
                    Settings.setValue("mediaItemTopMargin", mediaItemTopMarginL)
                    Settings.endGroup()
                }
            }
        }
        onXChanged: {
            if(mediaItemArea.pressed) {
            if (x == (parent.width - mediaItem.width) / 2) {
                xline.x = (parent.width) / 2
                xline.visible = true
            } else
                xline.visible = false
            }
        }
    }

    Text {
        z: 20
        id: timeText
        text: "17:30"
        font.pixelSize: 100
        font.family: mainFont.name
        font.weight: Font.Light
        color: "white"
        x: (parent.width - timeText.width) / 2
           + (isPortrait ? timeTextCenterOffsetP : timeTextCenterOffsetL)
        y: isPortrait ? timeTextTopMarginP : timeTextTopMarginL
        MouseArea {
            id: timeTextArea
            anchors.fill: parent
            drag.target: timeText
            drag.axis: Drag.XandYAxis
            onReleased: {
                xline.visible = false
                if (isPortrait) {
                    timeTextTopMarginP = parent.y
                    timeTextCenterOffsetP = (timeText.x - lockView.width / 2 + timeText.width / 2)
                    Settings.beginGroup("portrait")
                    Settings.setValue("timeTextTopMargin", timeTextTopMarginP)
                    Settings.setValue("timeTextCenterOffset", timeTextCenterOffsetP)
                    Settings.endGroup()
                } else {
                    timeTextTopMarginL = parent.y
                    timeTextCenterOffsetL = (timeText.x - lockView.width / 2 + timeText.width / 2)
                    Settings.beginGroup("landscape")
                    Settings.setValue("timeTextTopMargin", timeTextTopMarginL)
                    Settings.setValue(
                                "timeTextCenterOffset",timeTextCenterOffsetL)
                    Settings.endGroup()
                }
            }
        }
        onXChanged: {
            if(timeTextArea.pressed) {
            if (x == (parent.width - timeText.width) / 2) {
                xline.x = (parent.width) / 2
                xline.visible = true
            } else
                xline.visible = false
            }
        }
    }

    Text {
        z: 20
        id: dateText
        text: "10 February"
        font.pixelSize: 20
        //width: text.length
        font.family: mainFont.name
        font.weight: Font.Light
        smooth: true
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        x: (parent.width - dateText.width) / 2
           + (isPortrait ? dateTextCenterOffsetP : dateTextCenterOffsetL)
        y: isPortrait ? dateTextTopMarginP : dateTextTopMarginL
        MouseArea {
            id: dateTextArea
            anchors.fill: parent
            drag.target: dateText
            drag.axis: Drag.XandYAxis
            onReleased: {
                xline.visible = false
                if (isPortrait) {
                    dateTextTopMarginP = parent.y
                    dateTextCenterOffsetP = (dateText.x - lockView.width / 2 + dateText.width / 2)
                    Settings.beginGroup("portrait")
                    Settings.setValue("dateTextTopMargin", dateTextTopMarginP)
                    Settings.setValue("dateTextCenterOffset", dateTextCenterOffsetP)
                    Settings.endGroup()
                } else {
                    dateTextTopMarginL = parent.y
                    dateTextCenterOffsetL = (dateText.x - lockView.width / 2 + dateText.width / 2)
                    Settings.beginGroup("landscape")
                    Settings.setValue("dateTextTopMargin", dateTextTopMarginL)
                    Settings.setValue("dateTextCenterOffset", dateTextCenterOffsetL)
                    Settings.endGroup()
                }
            }
        }
        onXChanged: {
            if(dateTextArea.pressed) {
            if (x == (parent.width - dateText.width) / 2) {
                xline.x = (parent.width) / 2
                xline.visible = true
            } else
                xline.visible = false
            }
        }
    }

    Events {
        id: eventsItem
        x: isPortrait ? eventsItemLeftMarginP : eventsItemLeftMarginL
        y: isPortrait ? eventsItemTopMarginP : eventsItemTopMarginL
        MouseArea {
            anchors.fill: parent
            drag.target: eventsItem
            drag.axis: Drag.XandYAxis
            onReleased: {
                if (isPortrait) {
                    eventsItemLeftMarginP = parent.x
                    eventsItemTopMarginP = parent.y
                    Settings.beginGroup("portrait")
                    Settings.setValue("eventsItemLeftMargin", eventsItemLeftMarginP)
                    Settings.setValue("eventsItemTopMargin", eventsItemTopMarginP)
                    Settings.endGroup()
                } else {
                    eventsItemLeftMarginL = parent.x
                    eventsItemTopMarginL = parent.y
                    Settings.beginGroup("landscape")
                    Settings.setValue("eventsItemLeftMargin", eventsItemLeftMarginL)
                    Settings.setValue("eventsItemTopMargin", eventsItemTopMarginL)
                    Settings.endGroup()
                }
            }
        }
    }

    Item {
        id: batteryItem
        x: isPortrait ? batteryItemLeftMarginP : batteryItemLeftMarginL
        y: isPortrait ? batteryItemTopMarginP : batteryItemTopMarginL
        width: batteryImage.width + batteryText.width + batteryText.anchors.leftMargin
        height: batteryImage.height
        Image {
            id: batteryImage
            source: "file://" + appPath + "/icons/bat3.png"
            asynchronous: true
        }
        MouseArea {
            anchors.fill: parent
            drag.target: batteryItem
            drag.axis: Drag.XandYAxis
            onReleased: {
                if (isPortrait) {
                    batteryItemLeftMarginP = batteryItem.x
                    batteryItemTopMarginP = batteryItem.y
                    Settings.beginGroup("portrait")
                    Settings.setValue("batteryItemLeftMargin", batteryItemLeftMarginP)
                    Settings.setValue("batteryItemTopMargin", batteryItemTopMarginP)
                    Settings.endGroup()
                } else {
                    batteryItemLeftMarginL = batteryItem.x
                    batteryItemTopMarginL = batteryItem.y
                    Settings.beginGroup("landscape")
                    Settings.setValue("batteryItemLeftMargin", batteryItemLeftMarginL)
                    Settings.setValue("batteryItemTopMargin", batteryItemTopMarginL)
                    Settings.endGroup()
                }
            }
        }
        Text {
            id: batteryText
            text: "47%"
            font.pixelSize: 20
            font.family: mainFont.name
            font.weight: Font.Light
            color: "white"
            anchors {
                left: batteryImage.left
                leftMargin: 20
                verticalCenter: batteryImage.verticalCenter
                verticalCenterOffset: 2
            }
        }
    }

    Image {
        id: blueImage
        source: "file://" + appPath + "/icons/blueOn.png"
        visible: true
        x: isPortrait ? blueItemLeftMarginP : blueItemLeftMarginL
        y: isPortrait ? blueItemTopMarginP : blueItemTopMarginL
        MouseArea {
            anchors.fill: parent
            drag.target: blueImage
            drag.axis: Drag.XandYAxis
            onReleased: {
                if (isPortrait) {
                    blueItemLeftMarginP = blueImage.x
                    blueItemTopMarginP = blueImage.y
                    Settings.beginGroup("portrait")
                    Settings.setValue("blueItemLeftMargin", blueItemLeftMarginP)
                    Settings.setValue("blueItemTopMargin", blueItemTopMarginP)
                    Settings.endGroup()
                } else {
                    blueItemLeftMarginL = blueImage.x
                    blueItemTopMarginL = blueImage.y
                    Settings.beginGroup("landscape")
                    Settings.setValue("blueItemLeftMargin", blueItemLeftMarginL)
                    Settings.setValue("blueItemTopMargin", blueItemTopMarginL)
                    Settings.endGroup()
                }
            }
        }
    }

    Image {
        id: cellularImage
        source: "file://" + appPath + "/icons/sigLevel5.png"
        x: isPortrait ? cellularItemLeftMarginP : cellularItemLeftMarginL
        y: isPortrait ? cellularItemTopMarginP : cellularItemTopMarginL
        MouseArea {
            anchors.fill: parent
            drag.target: cellularImage
            drag.axis: Drag.XandYAxis
            onReleased: {
                if (isPortrait) {
                    cellularItemLeftMarginP = cellularImage.x
                    cellularItemTopMarginP = cellularImage.y
                    Settings.beginGroup("portrait")
                    Settings.setValue("cellularItemLeftMargin", cellularItemLeftMarginP)
                    Settings.setValue("cellularItemTopMargin", cellularItemTopMarginP)
                    Settings.endGroup()
                } else {
                    cellularItemLeftMarginL = cellularImage.x
                    cellularItemTopMarginL = cellularImage.y
                    Settings.beginGroup("landscape")
                    Settings.setValue("cellularItemLeftMargin", cellularItemLeftMarginL)
                    Settings.setValue("cellularItemTopMargin", cellularItemTopMarginL)
                    Settings.endGroup()
                }
            }
        }
    }
    Text {
        id: cellularText
        text: "2.5"
        font.pixelSize: 20
        font.family: mainFont.name
        font.weight: Font.Light
        color: "white"
        anchors {
            right: cellularImage.right
            rightMargin: 28
            verticalCenter: cellularImage.verticalCenter
            verticalCenterOffset: 2
        }
    }

    Image {
        id: profileImage
        source: "file://" + appPath + "/icons/profileGS.png"
        x: isPortrait ? profileItemLeftMarginP : profileItemLeftMarginL
        y: isPortrait ? profileItemTopMarginP : profileItemTopMarginL
        MouseArea {
            anchors.fill: parent
            drag.target: profileImage
            drag.axis: Drag.XandYAxis
            onReleased: {
                if (isPortrait) {
                    profileItemLeftMarginP = profileImage.x
                    profileItemTopMarginP = profileImage.y
                    Settings.beginGroup("portrait")
                    Settings.setValue("profileItemLeftMargin", profileItemLeftMarginP)
                    Settings.setValue("profileItemTopMargin", profileItemTopMarginP)
                    Settings.endGroup()
                } else {
                    profileItemLeftMarginL = profileImage.x
                    profileItemTopMarginL = profileImage.y
                    Settings.beginGroup("landscape")
                    Settings.setValue("profileItemLeftMargin", profileItemLeftMarginL)
                    Settings.setValue("profileItemTopMargin", profileItemTopMarginL)
                    Settings.endGroup()
                }
            }
        }
    }

    Rectangle {
        id: xline
        width: 1
        height: isPortrait ? 800 : 480
        visible: false
    }

    property int mediaItemLeftMarginL
    property int mediaItemTopMarginL
    property int timeTextCenterOffsetL
    property int timeTextTopMarginL
    property int dateTextCenterOffsetL
    property int dateTextTopMarginL
    property int eventsItemLeftMarginL
    property int eventsItemTopMarginL
    property int batteryItemLeftMarginL
    property int batteryItemTopMarginL
    property int blueItemLeftMarginL
    property int blueItemTopMarginL
    property int profileItemLeftMarginL
    property int profileItemTopMarginL
    property int cellularItemLeftMarginL
    property int cellularItemTopMarginL

    property int mediaItemLeftMarginP
    property int mediaItemTopMarginP
    property int timeTextCenterOffsetP
    property int timeTextTopMarginP
    property int dateTextCenterOffsetP
    property int dateTextTopMarginP
    property int eventsItemLeftMarginP
    property int eventsItemTopMarginP
    property int batteryItemLeftMarginP
    property int batteryItemTopMarginP
    property int blueItemLeftMarginP
    property int blueItemTopMarginP
    property int profileItemLeftMarginP
    property int profileItemTopMarginP
    property int cellularItemLeftMarginP
    property int cellularItemTopMarginP

    Component.onCompleted: {
        Settings.beginGroup("lockscreen")
        wallpaperPath = Settings.value("wallpaperPath",
                                       (appPath + "/icons/wallpaper.png"))

        Settings.endGroup()

        Settings.beginGroup("landscape")
        mediaItemLeftMarginL = Settings.value("mediaItemLeftMargin",
                                              (800 - mediaItem.width) / 2)
        mediaItemTopMarginL = Settings.value("mediaItemTopMargin",
                                             (480 - mediaItem.height) / 2 + 100)

        timeTextCenterOffsetL = Settings.value("timeTextCenterOffset", 0)
        timeTextTopMarginL = Settings.value("timeTextTopMargin", 40)
        dateTextCenterOffsetL = Settings.value("dateTextCenterOffset", 0)
        dateTextTopMarginL = Settings.value("dateTextTopMargin", 160)

        eventsItemLeftMarginL = Settings.value("eventsItemLeftMargin", 0)
        eventsItemTopMarginL = Settings.value(
                    "eventsItemTopMargin", (480 - eventsItem.height) / 2 + 60)
        batteryItemLeftMarginL = Settings.value("batteryItemLeftMargin", 38)
        batteryItemTopMarginL = Settings.value("batteryItemTopMargin", 30)
        blueItemLeftMarginL = Settings.value("blueItemLeftMargin", 32)
        blueItemTopMarginL = Settings.value("blueItemTopMargin", 140)
        profileItemLeftMarginL = Settings.value("profileItemLeftMargin", 740)
        profileItemTopMarginL = Settings.value("profileItemTopMargin", 140)
        cellularItemLeftMarginL = Settings.value("cellularItemLeftMargin", 736)
        cellularItemTopMarginL = Settings.value("cellularItemTopMargin", 30)

        Settings.endGroup()

        Settings.beginGroup("portrait")
        mediaItemLeftMarginP = Settings.value("mediaItemLeftMargin",
                                              (480 - mediaItem.width) / 2)
        mediaItemTopMarginP = Settings.value("mediaItemTopMargin",
                                             (800 - mediaItem.height) / 2 + 150)

        timeTextCenterOffsetP = Settings.value("timeTextCenterOffset", 0)
        timeTextTopMarginP = Settings.value("timeTextTopMargin", 40)
        dateTextCenterOffsetP = Settings.value("dateTextCenterOffset", 0)
        dateTextTopMarginP = Settings.value("dateTextTopMargin", 160)

        eventsItemLeftMarginP = Settings.value("eventsItemLeftMargin", 0)
        eventsItemTopMarginP = Settings.value(
                    "eventsItemTopMargin", (800 - eventsItem.height) / 2 - 130)
        batteryItemLeftMarginP = Settings.value("batteryItemLeftMargin", 38)
        batteryItemTopMarginP = Settings.value("batteryItemTopMargin", 30)
        blueItemLeftMarginP = Settings.value("blueItemLeftMargin", 32)
        blueItemTopMarginP = Settings.value("blueItemTopMargin", 140)
        profileItemLeftMarginP = Settings.value("profileItemLeftMargin", 420)
        profileItemTopMarginP = Settings.value("profileItemTopMargin", 140)
        cellularItemLeftMarginP = Settings.value("cellularItemLeftMargin", 416)
        cellularItemTopMarginP = Settings.value("cellularItemTopMargin", 30)

        Settings.endGroup()

        xline.visible = false
    }
}
