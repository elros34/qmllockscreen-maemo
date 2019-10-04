import QtQuick 1.1

Item {
    width: parent.width
    height: parent.height

    LockView {

    }

    FileSelect {
        id: fileSelect
        visible: false
        z: 40
    }

}
