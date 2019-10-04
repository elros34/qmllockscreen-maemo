import QtQuick 1.1

Rectangle {
    id: tmenu
    height: parent.height
    color: "black"
    property int mItemHeight
    anchors {
        bottom: parent.top
        left: parent.left
        right: parent.right
    }


    Item {
        id: columnI
        anchors.fill: parent

        TopMenuItem {
            height: mItemHeight
            text: "General profile"
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                bottomMargin: mItemHeight / 3
            }
        }

        TopMenuItem {
            height: mItemHeight
            text: "Offline mode"
            anchors {
                left: parent.left
                right: parent.right
                bottom: columnI.children[0].top
            }
        }
        TopMenuItem {
            height: mItemHeight
            text: "Phone"
            anchors {
                left: parent.left
                right: parent.right
                bottom: columnI.children[1].top
            }
        }
    }
    function highlightItem(i) {
        if (i >= columnI.children.length)
            return

        if (i == -1) {
            columnI.children[0].textColor = "grey"
        } else {
            for (var j = 0; j < columnI.children.length; ++j) {
                if (i != j)
                    columnI.children[j].textColor = "grey"
            }
            columnI.children[i].textColor = "white"
        }
    }

    Connections {
        target: uiTools
        onProfileChanged: {
            if (is_general) {
                columnI.children[0].text = "Silent profile"
            } else {
                columnI.children[0].text = "General profile"
            }

            console.log("is general" + is_general)
        }
        onCellularRadioChanged: {
            if(radioState == "online") {
                columnI.children[1].text = "Offline mode"
            }
            else if(radioState == "activation_pending") {
                columnI.children[1].text = "Offline mode ((*))"
            }
            else if(radioState == "inactive") {
                columnI.children[1].text = "Online mode"
            }
        }
    }
}
