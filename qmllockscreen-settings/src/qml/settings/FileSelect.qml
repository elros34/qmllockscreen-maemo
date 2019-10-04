import QtQuick 1.1

Rectangle {
    width: parent.width
    height: parent.height
    color: "black"
    signal fileSelected(string path)

    MouseArea {
        anchors.fill: parent
        onClicked: {
            fileSelected("")
        }
    }

    Rectangle {
        width: parent.width
        height: parent.height - 56
        color: "black"
        anchors.bottom: parent.bottom


        GridView {
            anchors.fill: parent
            model: fileModel
            cellWidth: 80

            delegate: FileItem {
                onClicked: {
                    if(isDir)
                        fileModel.setDir(path)
                    else
                        fileSelected(path)
                }
            }
        }
    }

}
