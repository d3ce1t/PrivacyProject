import QtQuick 2.0

Rectangle {
    id: simplebutton
    width: 150
    height: 50

    property color buttonColor: "white"
    property color onHoverColor: "gold"
    property color borderColor: "white"
    property string label: "Default text"

    signal buttonClick()

    onButtonClick: {
        console.log(buttonLabel.text + " clicked" )
    }

    Text{
        id: buttonLabel
        anchors.centerIn: parent
        text: simplebutton.label
    }

    MouseArea{
        id: buttonMouseArea
        anchors.fill: parent
        onClicked: buttonClick()
        hoverEnabled: true
        onEntered: parent.border.color = onHoverColor
        onExited: parent.border.color = borderColor
    }

    //determines the color of the button by using the conditional operator
    color: buttonMouseArea.pressed ? Qt.darker(buttonColor, 1.5) : buttonColor
}
