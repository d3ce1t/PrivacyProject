import QtQuick 2.0


Item {
    id: customItem
    width: background.width
    height: background.height

    property color color: "transparent"
    property string label: "Default text"
    property bool value: false

    signal click()


    onClick: {
        if (value == false) {
            value = true
        } else {
            value = false
        }
    }

    // Background
    Rectangle {
        id: background
        anchors.left:  checkBoxLabel.left;
        anchors.top:   checkBoxRect.top;
        anchors.right: checkBoxRect.right;
        anchors.bottom: checkBoxRect.bottom;
        anchors.margins: -5
        color: customItem.color
    }

    // Label
    Text {
        id: checkBoxLabel
        text: label
    }

    // Check box state
    Rectangle {
        id: checkBoxRect
        anchors.left: checkBoxLabel.right
        anchors.verticalCenter: checkBoxLabel.verticalCenter
        anchors.margins: 10
        width: 25
        height: 25
        color: value == true? "black" : "white"
        border.width: 1
        border.color: value == true? "blue" : "black"
    }

    // Mouse Area
    MouseArea{
        id: checkBoxMouseArea
        anchors.fill: checkBoxRect
        onClicked: click()
        //hoverEnabled: true
        //onEntered: parent.border.color = onHoverColor
        //onExited: parent.border.color = borderColor
    }
}
