import QtQuick 2.0

Item {
    id: wrapper
    width: 640
    height: 480
    focus: true

    DropArea {
        id: dragTarget
        anchors.fill: parent

        onDropped: {
            winObject.processListItem(drop.source)
        }

        Rectangle {
            id: dropRectangle
            anchors.fill: parent
            color: Qt.rgba(0.5, 0.7, 1.0, 0.5)
            opacity: 0;

            states: [
                State {
                    when: dragTarget.containsDrag
                    PropertyChanges {
                        target: dropRectangle
                        opacity: 1;
                    }
                }
            ]
        }
    }

    // Settings
    Rectangle {
        id: settings
        color: Qt.rgba(0, 0.7, 1, 0.7)
        radius: 15
        border.width: 1
        border.color: "black"
        anchors.fill: wrapper
        anchors.margins: 30
        visible: false

        Column {
            id: settingsLeft
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.margins: 20
            spacing: 5

            Checkbox {
                id: checkBoxXYRot
                anchors.right: parent.right
                label: "Enable XY rotation"
                value: true
                onClick : {
                    checkBoxZRot.value = false
                }
            }

            Checkbox {
                id: checkBoxZRot
                anchors.right: parent.right
                label: "Enable ZY rotation"
                value: false
                onClick : {
                    checkBoxXYRot.value = false
                }
            }

            Checkbox {
                id: checkBoxYTrans
                anchors.right: parent.right
                label: "Enable Y translation"
                value: false
            }
        }
    }

    // Draw FPS
    Rectangle {
        color: Qt.rgba(0, 0.7, 1, 0.5)
        radius: 10
        anchors.fill: textFrameId
        anchors.margins: -10
    }

    Text {
        id: textFrameId
        text: Math.round(winObject.fps) + " fps"
        color: "black"
        wrapMode: Text.WordWrap
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_M) {
            settings.visible = !settings.visible;
            event.accepted = true;
        }
        else if (event.key === Qt.Key_Escape) {
            Qt.quit();
        }
        else if (event.key === Qt.Key_Up) {
            if (checkBoxXYRot.value === true) {
                winObject.rotateAxisX(2)
            } else if (checkBoxZRot.value === true) {
                winObject.rotateAxisX(-2)
            }
        }
        else if (event.key === Qt.Key_Down) {
            if (checkBoxXYRot.value === true) {
                winObject.rotateAxisX(-2)
            } else if (checkBoxZRot.value === true) {
                winObject.rotateAxisX(+2)
            }
        }
        else if (event.key === Qt.Key_Right) {
            if (checkBoxXYRot.value === true) {
                winObject.rotateAxisY(-2)
            } else if (checkBoxZRot.value === true) {
                winObject.rotateAxisZ(-2)
            }
        }
        else if (event.key === Qt.Key_Left) {
            if (checkBoxXYRot.value === true) {
                winObject.rotateAxisY(2)
            } else if (checkBoxZRot.value === true) {
                winObject.rotateAxisZ(2)
            }
        }
        else if (event.key === Qt.Key_W) {
            if (checkBoxYTrans.value === false)
                winObject.translateAxisZ(-0.1)
            else
                winObject.translateAxisY(0.1)
        }
        else if (event.key === Qt.Key_S) {
            if (checkBoxYTrans.value === false)
                winObject.translateAxisZ(0.1)
            else
                winObject.translateAxisY(-0.1)
        }
        else if (event.key === Qt.Key_A) {
            winObject.translateAxisX(-0.1)
        }
        else if (event.key === Qt.Key_D) {
            winObject.translateAxisX(0.1)
        }
        else if (event.key === Qt.Key_R) {
            winObject.resetPerspective()
        }
    }

    Keys.onReturnPressed: {
        console.log(skeleton.state)
        console.log("Width: " + wrapper.width + " Height: " + wrapper.height)
    }
}
