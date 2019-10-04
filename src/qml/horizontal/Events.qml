import QtQuick 1.1

Rectangle {
    width: 100
    height: 125
    color: "transparent"

    Image {
        id: call
        source: "file://" + appPath + "/icons/call.png"
        opacity: 0.7
        visible: callCount != 0
        anchors {
            left: parent.left
            leftMargin: 15
        }
    }
    Text {
        id: callText
        text: callCount
        font.pixelSize: 22
        font.family: mainFont.name
        font.weight: Font.Light
        color: "white"
        visible: call.visible
        anchors {
            left: call.right
            leftMargin: 10
            verticalCenter: call.verticalCenter
        }
    }
    Image {
        id: sms
        source: "file://" + appPath + "/icons/sms.png"
        opacity: 0.7
        visible: smsCount != 0
        anchors {
            top: call.bottom
            topMargin: 40
            left: parent.left
            leftMargin: 15
        }
    }
    Text {
        id: smsText
        text: smsCount
        font.pixelSize: 22
        font.family: mainFont.name
        font.weight: Font.Light
        visible: sms.visible
        color: "white"
        anchors {
            left: sms.right
            leftMargin: 10
            verticalCenter: sms.verticalCenter
        }
    }

    property int callCount: 0
    property int smsCount: 0

    Connections {
        target: uiTools
        onCallEvent: {
            if(msg == "clear")
               callCount = 0
            else
                ++callCount
        }
        onSmsEvent: {
            if(msg == "clear")
                smsCount = 0
            else
                ++smsCount
        }
    }
    Component.onCompleted:
    {
        ++callCount
        ++smsCount
    }
}
