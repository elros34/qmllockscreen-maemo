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

    Behavior on y {
        id: yBehavior
        NumberAnimation {
            id: sAnim
        }
    }
    Behavior on x {
        id: xBehavior
        NumberAnimation {
            id: sxAnim
            onRunningChanged: {
                if (!sxAnim.running && (lockView.x == -lockView.width
                                        || lockView.x == lockView.width)) {
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
    property string barPos
    onXChanged: {
        if (x > 60 && x < 250 && !isqmode) {
            quickBarTimer.start()
            barPos = "left"
        } else if (x < -60 && x > -250) {
            quickBarTimer.start()
            barPos = "right"
        } else {
            quickBarTimer.stop()
        }
    }

    Timer {
        id: quickBarTimer
        interval: 800
        onTriggered: {
            if (barPos == "left") {
                lockViewArea.drag.maximumX = 100
                lockViewArea.drag.minimumX = 100
                x = 100
                quickBar.x = 0
            } else {

                //right
                lockViewArea.drag.maximumX = -100
                lockViewArea.drag.minimumX = -100
                x = -100
                quickBar.x = parent.width - quickBar.width
            }

            isqmode = true
            lockView.showQuickBar()
            quickBarTimer.stop()
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

            xBehavior.enabled = false
            lockView.x = 0
            xBehavior.enabled = true
        }
    }

    Image {
        rotation: isPortrait ? 0 : -90
        source: "file://" + appPath + "/icons/wallpaper.png"
        anchors.centerIn: parent
    }

    MouseArea {
        id: lockViewArea
        anchors.fill: parent
        drag.target: parent
        drag.axis: Drag.YAxis
        drag.maximumY: parent.height
        property bool brmode: false
        property int ypos: 0
        onPressed: {
            if (isqmode) {
                lockView.hideQuickBar()
                isqmode = false
            } else {

                if (mouseY < 40) {
                    lockViewArea.drag.minimumX = 0
                    lockViewArea.drag.maximumX = 0
                    lockViewArea.drag.minimumY = 0
                    lockViewArea.drag.maximumY = 0
                    brmode = true
                    ypos = mouseY
                } else if (mouseY >= 40 && mouseY < 150) {
                    lockViewArea.drag.minimumX = 0
                    lockViewArea.drag.maximumX = 0
                    lockViewArea.drag.minimumY = 0
                    lockViewArea.drag.maximumY = parent.height
                } else {
                    lockViewArea.drag.minimumX = -parent.height
                    lockViewArea.drag.maximumX = parent.height
                    lockViewArea.drag.minimumY = 0
                    lockViewArea.drag.maximumY = 0
                }
            }

            lockView.mainPressed()
        }
        onReleased: {
            if (!isqmode) {
                if (parent.x < -parent.width / 3) {
                    parent.x = -parent.width
                } else if (parent.x > -parent.width / 3
                           && parent.x < parent.width / 3) {
                    parent.x = 0
                } else {
                    parent.x = parent.width
                }
                parent.y = 0

                if (parent.y > 0) {
                    if (j == 0) // Profile
                    {
                        profileTimer.start()
                        //console.log("1 menu")
                    } else if (j == 1) //gsm mode
                    {
                        uiTools.toggleCellularRadio()
                        //console.log("2 menu")
                    } else if (j == 2) //Phone
                    {
                        uiTools.openPhoneApp()
                        // console.log("3 menu")
                    }
                }

                _j = -1

                drag.maximumY = parent.height
                ypos = 0
            }
            brmode = false
            drag.maximumX = parent.width
            drag.minimumX = -parent.width
            drag.maximumY = 0
            drag.minimumY = 0
        }
        onMousePositionChanged: {

            if (brmode) {
                if (mouseY - ypos > 70) {
                    uiTools.disableBrightness()
                    brmode = false
                }
            } else {

                _j = (lockView.y - topMenuItem) / topMenuItem - 0.5

                if (j != _j && _j >= -1 && _j < 3) {
                    j = _j

                    topMenu.highlightItem(j)
                    if (j >= 0) {
                        uiTools.tickVibrate()
                    }
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
        Rectangle {
            width: 3
            height: 360
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: topMenu.bottom
            anchors.topMargin: -180
            radius: 10
            opacity: 0.7
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
        id: profileImage
        source: "file://" + appPath + "/icons/profileGS.png"
        asynchronous: true
        anchors {
            right: parent.right
            rightMargin: 38
            top: parent.top
            topMargin: 30
        }
    }
    Image {
        id: batteryImage
        source: "file://" + appPath + "/icons/bat3.png"
        asynchronous: true
        anchors {
            left: parent.left
            leftMargin: 38
            top: parent.top
            topMargin: 30
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
            topMargin: 40
        }
    }
    Text {
        z: 20
        id: dateText
        text: "10 February"
        font.pixelSize: 20
        width: text.length
        font.family: mainFont.name
        font.weight: Font.Light
        smooth: true
        color: "white"
        horizontalAlignment: Text.AlignHCenter
        anchors {
            top: timeText.bottom
            topMargin: -15
            horizontalCenter: timeText.horizontalCenter
        }
    }

    MediaItem {
        z: 10
        anchors {
            centerIn: parent
            verticalCenterOffset: isPortrait ? 150 : 100
        }
    }

    Events {
        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: isPortrait ? -100 : 0
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
                lockView.x = 0
                if(isqmode)
                {
                    lockView.hideQuickBar()
                    isqmode = false
                    lockViewArea.drag.minimumX = -parent.height
                    lockViewArea.drag.maximumX = parent.height
                    lockViewArea.drag.minimumY = 0
                    lockViewArea.drag.maximumY = 0
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
        onShowSignal: {
            timeText.text = Qt.formatTime(new Date())
        }

        onRealShowSignal: {
            dateText.text = uiTools.getCurrentDate()
        }
    }

    property int better_sense
    Connections {
        target: mainLock
    }
    Component.onCompleted: {
        Settings.beginGroup("lockscreen")

        better_sense = Settings.value("better_sense", 1)
        quickBarTimer.interval = Settings.value("quickBar_delay", 800)

        if (better_sense == 1) {
            lockViewArea.drag.axis = Drag.XandYAxis
            lockViewArea.drag.maximumY = 0
            lockViewArea.drag.minimumY = 0
        }

        Settings.setValue("better_sense", better_sense)
        Settings.setValue("quickBar_delay", quickBarTimer.interval)

        Settings.endGroup()
    }
}
