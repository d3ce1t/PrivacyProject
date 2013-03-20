import QtQuick 2.0

Item {
    id: wrapper
    width: 640
    height: 480
    focus: true

    Rectangle {
        id: settings
        color: Qt.rgba(0, 0.7, 1, 0.7)
        radius: 15
        border.width: 1
        border.color: "black"
        anchors.fill: wrapper
        anchors.margins: 30
        visible: false

        Text {
            id: label
            color: "white"
            wrapMode: Text.WordWrap
            text: "The background here is a squircle rendered with raw OpenGL using the 'beforeRender()' signal in QQuickWindow. This text label and its border is rendered using QML"
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20
        }

        Column {
            anchors.top: label.bottom
            anchors.left: parent.left
            anchors.margins: 20
            spacing: 5

            Button {
                id: button1
                label: "DrawBoundingBox"
                buttonColor: rootItem.drawBoundingBox ? "green" : "blue"
                onButtonClick: {
                    rootItem.drawBoundingBox = !rootItem.drawBoundingBox
                }
            }

            Button {
                id: button2
                label: "DrawCenterOfMass"
            }
        }
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_M) {
            settings.visible = !settings.visible;
            event.accepted = true;
        }

        if (event.key == Qt.Key_Escape) {
            Qt.quit();
        }
    }

    Keys.onReturnPressed: console.log("Pressed return " + wrapper.width + "x" + wrapper.height);
}
