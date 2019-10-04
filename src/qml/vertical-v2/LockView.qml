import QtQuick 1.1

Item {
    id: lockView
    width: isPortrait ? 480 : 800
    height: isPortrait ? 800 : 480
    property int imHeight: lockView.height + shadowIm.height
    signal mainPressed
    property int j: 0
    property int _j: 0
    property int topMenuItem: 100

    property string wallpaperPath

    Behavior on y {
        id: yBehavior
        NumberAnimation {
            id: sAnim
            onRunningChanged: {
                if (!sAnim.running && lockView.y == -imHeight) {
                    mainLock.hideLockScreen()
                    console.log("hide")
                    resetTimer.start() //TODO
                }
            }
        }
    }

    property bool isqmode: false
    signal showQuickBar
    signal hideQuickBar
    onYChanged: {
        if (y > -250 && y < -60 && !isqmode) {
            quickBarTimer.start()
        } else {
            quickBarTimer.stop()
        }

        _j = (lockView.y - topMenuItem) / topMenuItem - 0.5

        if (j != _j && _j >= -1 && _j < 3) {
            j = _j

            if(tArea.pressed) {
                topMenu.highlightItem(j)
                    if (j >= 0) {
                          uiTools.tickVibrate()
            }
          }
        }
    }

    Timer {
        id: quickBarTimer
        interval: 800
        onTriggered: {
            quickBarTimer.stop()
            lockViewArea.drag.maximumY = -100
            lockViewArea.drag.minimumY = -100
            y = -100
            isqmode = true
            lockView.showQuickBar()
        }
    }

    Timer {
        id: resetTimer
        running: false
        interval: 1000
        onTriggered: {
            yBehavior.enabled = false
            lockView.y = 0
            yBehavior.enabled = true
        }
    }

    Image {
        rotation: isPortrait ? 0 : -90
        source: "file://" + wallpaperPath
        anchors.centerIn: parent
    }

    MouseArea {
        id: lockViewArea
        anchors.fill: parent
        drag.target: parent
        drag.axis: Drag.YAxis
        drag.maximumY: 0
        property int ypos: 0
        onPressed: {
            if (isqmode) {
                lockView.hideQuickBar()
                isqmode = false
            }
            lockView.mainPressed()
        }
        onReleased: {
            if (!isqmode) {
                if (parent.y < -parent.height / 3) { //unlock
                    parent.y = -parent.height - shadowIm.height
                } else if (parent.y > -parent.height / 3) {
                    parent.y = 0
                }

                drag.maximumY = parent.height
                ypos = 0
            }
            drag.maximumY = 0
            drag.minimumY = -parent.height
        }


        Item {
            id: brightnessActivator
            width: parent.width
            height: 80
            y: 0
            property bool brightnessMode: false
            onYChanged: {
                if(brightnessMode)
                    return

                if(y > 70)
                {
                    brightnessMode = true
                    uiTools.disableBrightness()
                    console.log("disable brightness")
                }
            }

            MouseArea {
                id: bArea
                anchors.fill: parent
                drag.target: parent
                drag.axis: Drag.YAxis
                property int yoffset: 0
                onPressed: {
                    brightnessActivator.brightnessMode = false;
                    yoffset = mouseY
                }
                onReleased: {
                    brightnessActivator.brightnessMode = false

                    brightnessActivator.y = 0
                }
            }

        }

        Item {
            id: topMenuActivator
            width: parent.width
            height: isPortrait ? 400 : 225
            y: 80
            property bool topMenuActive: false
            onYChanged: {
                if(topMenuActive)
                {
                    lockView.y = y + tArea.yoffset - 50
                    return
                }

                if((y + tArea.yoffset) <= 50)
                {
                    topMenuActive = true
                    yBehavior.enabled = false
                    tArea.drag.minimumY = y
                    indBar.visible = true
                }
            }

            MouseArea {
                id: tArea
                anchors.fill: parent
                drag.target: parent
                drag.axis: Drag.YAxis
                property int yoffset: 0
                onPressed: {
                    yBehavior.enabled = false
                    yoffset = mouseY
                }
                onReleased: {
                    if (lockView.y > 0) {
                        if (j == 0) // Profile
                        {
                            profileTimer.start()
                        } else if (j == 1) //gsm mode
                        {
                            uiTools.toggleCellularRadio()
                        } else if (j == 2) //Phone
                        {
                            uiTools.openPhoneApp()
                        }
                    }

                    _j = -1


                    topMenuActivator.topMenuActive = false
                    parent.y = 80
                    tArea.drag.minimumY = -400
                    yBehavior.enabled = true
                    lockView.y = 0
                    indBar.visible = false
                }
            }

        }
    }




    Timer {
        id: profileTimer
        running: false
        interval: 250
        onTriggered: uiTools.toggleProfile()
    }

    Timer {
        id: tickTimer
        running: false
        interval: 10
        onTriggered: uiTools.tickVibrate()
    }

    TopMenu {
        id: topMenu
        mItemHeight: topMenuItem
        anchors {
            bottom: parent.top
            left: parent.left
            right: parent.right
        }
        Rectangle {
            id: indBar
            width: 3
            height: 360
            visible: false
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: topMenu.bottom
            anchors.topMargin: -179
            radius: 10
            opacity: 0.9
            rotation: -90
            gradient: Gradient {
                GradientStop {
                    position: 0.00
                    color: "transparent"
                }
                GradientStop {
                    position: 0.5
                    color: "lightgreen"
                }
                GradientStop {
                    position: 1.00
                    color: "transparent"
                }
            }
        }
    }

    Image {
        id: shadowIm
        source: "file://" + appPath + "/icons/shadow.png"
        anchors.top: parent.bottom
    }

    Image {
        id: batteryImage
        source: "file://" + appPath + "/icons/bat3.png"
        asynchronous: false
        anchors {
            left: parent.left
            leftMargin: isPortrait ? batteryItemLeftMarginP : batteryItemLeftMarginL
            top: parent.top
            topMargin: isPortrait ? batteryItemTopMarginP : batteryItemTopMarginL
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

    Image {
        id: blueImage
        source: "file://" + appPath + "/icons/blueOn.png"
        asynchronous: false
       // visible: false
        anchors {
            left: parent.left
            leftMargin: isPortrait ? blueItemLeftMarginP : blueItemLeftMarginL
            top: parent.top
            topMargin: isPortrait ? blueItemTopMarginP : blueItemTopMarginL
        }
    }


    Image {
        id: cellularImage
        source: "file://" + appPath + "/icons/sigLevel5.png"
        asynchronous: false
        anchors {
            left: parent.left
            leftMargin: isPortrait ? cellularItemLeftMarginP : cellularItemLeftMarginL
            top: parent.top
            topMargin: isPortrait ? cellularItemTopMarginP : cellularItemTopMarginL
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
        asynchronous: false
        anchors {
            left: parent.left
            leftMargin: isPortrait ? profileItemLeftMarginP : profileItemLeftMarginL
            top: parent.top
            topMargin: isPortrait ? profileItemTopMarginP : profileItemTopMarginL
        }
    }

    Text {
        z: 20
        id: timeText
        text: "10:24"
        font.pixelSize: 100
        font.family: mainFont.name
        font.weight: Font.Light
        color: "white"
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            horizontalCenterOffset: isPortrait ? timeTextCenterOffsetP : timeTextCenterOffsetL
            topMargin: isPortrait ? timeTextTopMarginP : timeTextTopMarginL
        }
    }
    Text {
        z: 20
        id: dateText
        text: "10 February"
        font.pixelSize: 20
       // width: text.length
        font.family: mainFont.name
        font.weight: Font.Light
        smooth: true
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            horizontalCenterOffset: isPortrait ? dateTextCenterOffsetP : dateTextCenterOffsetL
            topMargin: isPortrait ? dateTextTopMarginP : dateTextTopMarginL
        }
    }

    MediaItem {
        z: 10
        id: mediaItem
        anchors {
            left: parent.left
            top: parent.top
            leftMargin: isPortrait ? mediaItemLeftMarginP : mediaItemLeftMarginL
            topMargin: isPortrait ? mediaItemTopMarginP : mediaItemTopMarginL
        }
    }

    Events {
        id: eventsItem

        anchors {
            left: parent.left
            top: parent.top
            leftMargin: isPortrait ? eventsItemLeftMarginP : eventsItemLeftMarginL
            topMargin: isPortrait ? eventsItemTopMarginP : eventsItemTopMarginL
        }
    }

    Item {
        z: 100
        anchors.fill: parent
        visible: false
        id: lockImage
        Image {
            source: "file://" + appPath + "/icons/lock.png"
            anchors.centerIn: parent
            opacity: 0.5
            z: 100
        }
        MouseArea {
            anchors.fill: parent
            property bool brmode: false
            property int ypos: 0
            onPressed: {
                if (mouseY < 70) {
                    brmode = true
                    ypos = mouseY
                }
            }
            onMousePositionChanged: {
                if (brmode) {
                    if (mouseY - ypos > 40) {
                        uiTools.disableBrightness()
                        brmode = false
                    }
                }
            }
        }
    }

    Connections {
        target: uiTools
        onSetMouseEvents: {
            if (enable)
            {
                lockViewArea.enabled = true
                lockImage.visible = false
            }
            else
            {
                lockViewArea.enabled = false
                lockImage.visible = true
                lockView.y = 0
                if(isqmode)
                {
                    lockView.hideQuickBar()
                    isqmode = false
                    lockViewArea.drag.maximumY = parent.height
                    lockViewArea.drag.minimumY = -parent.height
                }
            }
        }
        onProfileChanged: {
            if (is_general) {
                profileImage.source = "file://" + appPath + "/icons/profileG.png"
            } else {
                profileImage.source = "file://" + appPath + "/icons/profileGS.png"
            }

            console.log("is general" + is_general)
        }
        onBatteryChanged: {
            batteryText.text = bp + "%"
            if (bp > 83)
                batteryImage.source = "file://" + appPath + "/icons/bat5.png"
            else if (bp > 67)
                batteryImage.source = "file://" + appPath + "/icons/bat4.png"
            else if (bp > 50)
                batteryImage.source = "file://" + appPath + "/icons/bat3.png"
            else if (bp > 33)
                batteryImage.source = "file://" + appPath + "/icons/bat2.png"
            else if (bp > 16)
                batteryImage.source = "file://" + appPath + "/icons/bat1.png"
            else
                batteryImage.source = "file://" + appPath + "/icons/bat0.png"
        }
        onBlueToothChanged: {
            blueImage.visible = enabled
        }
        onCellularRadioChanged: {
            if(radioState == "online") {
                cellularImage.visible = true
                cellularText.visible = true
            }
            else {
                cellularImage.visible = false
                cellularText.visible = false
            }
        }
        onSignalLevelChanged: {
            if (level > 90)
                cellularImage.source = "file://" + appPath + "/icons/sigLevel7.png"
            if (level > 80)
                cellularImage.source = "file://" + appPath + "/icons/sigLevel6.png"
            else if (level > 70)
                cellularImage.source = "file://" + appPath + "/icons/sigLevel5.png"
            else if (level > 55)
                cellularImage.source = "file://" + appPath + "/icons/sigLevel4.png"
            else if (level > 45)
                cellularImage.source = "file://" + appPath + "/icons/sigLevel3.png"
            else if (level > 30)
                cellularImage.source = "file://" + appPath + "/icons/sigLevel2.png"
            else if (level > 10)
                cellularImage.source = "file://" + appPath + "/icons/sigLevel1.png"
            else
                cellularImage.source = "file://" + appPath + "/icons/sigLevel0.png"
        }
        onRatChanged: {
            if(value == 0) {
                cellularText = ""
            } else if(value == 1) {
                cellularText = "2.5"
            } else if(value == 2) {
                cellularText = "3"
            }
        }

        onShowSignal: {
            timeText.text = Qt.formatTime(new Date())
        }

        onRealShowSignal: {
            dateText.text = uiTools.getCurrentDate()
        }

        //        onOffSignal: { //wrong way
        //            yBehavior.enabled = false
        //            lockView.y = 0
        //            yBehavior.enabled = true
        //        }
    }

    property int better_sense
    Connections {
        target: mainLock
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

        better_sense = Settings.value("better_sense", 1)
        quickBarTimer.interval = Settings.value("quickBar_delay", 800)

        if (better_sense == 1) {
            console.log("better sense activated")
            lockViewArea.drag.axis = Drag.XandYAxis
            lockViewArea.drag.maximumX = 0
            lockViewArea.drag.minimumX = 0
        }

        Settings.setValue("better_sense", better_sense)
        Settings.setValue("quickBar_delay", quickBarTimer.interval)

        //ui
        wallpaperPath = Settings.value("wallpaperPath", (appPath + "/icons/wallpaper.png"))
        Settings.endGroup()


        Settings.beginGroup("landscape")
        mediaItemLeftMarginL = Settings.value("mediaItemLeftMargin", (800-mediaItem.width)/2)
        mediaItemTopMarginL = Settings.value("mediaItemTopMargin", (480-mediaItem.height)/2 + 100)

        timeTextCenterOffsetL = Settings.value("timeTextCenterOffset", 0)
        timeTextTopMarginL = Settings.value("timeTextTopMargin", 40)
        dateTextCenterOffsetL = Settings.value("dateTextCenterOffset", 0)
        dateTextTopMarginL = Settings.value("dateTextTopMargin", 160)

        eventsItemLeftMarginL = Settings.value("eventsItemLeftMargin", 0)
        eventsItemTopMarginL = Settings.value("eventsItemTopMargin", (480 - eventsItem.height)/2 + 60)
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
        mediaItemLeftMarginP = Settings.value("mediaItemLeftMargin", (480-mediaItem.width)/2)
        mediaItemTopMarginP = Settings.value("mediaItemTopMargin", (800-mediaItem.height)/2 + 150 )

        timeTextCenterOffsetP = Settings.value("timeTextCenterOffset", 0)
        timeTextTopMarginP = Settings.value("timeTextTopMargin", 40)
        dateTextCenterOffsetP = Settings.value("dateTextCenterOffset", 0)
        dateTextTopMarginP = Settings.value("dateTextTopMargin", 160)

        eventsItemLeftMarginP = Settings.value("eventsItemLeftMargin", 0)
        eventsItemTopMarginP = Settings.value("eventsItemTopMargin", (800 - eventsItem.height)/2 - 130)
        batteryItemLeftMarginP = Settings.value("batteryItemLeftMargin", 38)
        batteryItemTopMarginP = Settings.value("batteryItemTopMargin", 30)
        blueItemLeftMarginP = Settings.value("blueItemLeftMargin", 32)
        blueItemTopMarginP = Settings.value("blueItemTopMargin", 140)
        profileItemLeftMarginP = Settings.value("profileItemLeftMargin", 420)
        profileItemTopMarginP = Settings.value("profileItemTopMargin", 140)
        cellularItemLeftMarginP = Settings.value("cellularItemLeftMargin", 416)
        cellularItemTopMarginP = Settings.value("cellularItemTopMargin", 30)

        Settings.endGroup()
    }
}
