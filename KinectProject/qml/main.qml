import QtQuick 2.0
import QtQuick.Controls 1.0
import OpenGLUnderQML 1.0

ApplicationWindow {
    id: mainWindow
    minimumWidth: 640
    minimumHeight: 480
    width: instanceViewer.width
    height: instanceViewer.height

    InstanceViewer {
        id: instanceViewer
        objectName: "viewer"
        anchors.fill: parent
        width: 640
        height: 480
        focus: true

        // Draw FPS
        Rectangle {
            color: Qt.rgba(0, 0.7, 1, 0.5)
            radius: 10
            anchors.fill: textFrameId
            anchors.margins: -10
        }

        Text {
            id: textFrameId
            text: Math.round(viewerWindow.fps) + " fps"
            color: "black"
            font.pixelSize: 11
            wrapMode: Text.WordWrap
            anchors.left: instanceViewer.left
            anchors.top: instanceViewer.top
            anchors.margins: 20
        }

        // Drop Area
        DropArea {
            id: dragTarget
            anchors.fill: instanceViewer

            onDropped: {
                viewerWindow.processListItem(drop.source)
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

        // Settings Overlay
        Rectangle {
            id: settings
            color: Qt.rgba(0, 0.7, 1, 0.7)
            radius: 15
            anchors.fill: instanceViewer
            anchors.margins: 30
            visible: false

            ExclusiveGroup {
                id: rotationOptions
            }

            GroupBox {
                title: "Moving Options"
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 20

                Column {
                    spacing: 5

                    RadioButton {
                        id: checkBoxXYRot
                        text: "Enable XY rotation"
                        checked: true
                        exclusiveGroup: rotationOptions
                    }
                    RadioButton {
                        id: checkBoxZRot
                        text: "Enable ZY rotation"
                        exclusiveGroup: rotationOptions
                    }
                    CheckBox {
                        id: checkBoxYTrans
                        text: "Enable Y translation"
                    }
                }
            }

            ExclusiveGroup {
                id: privacyFilter
            }

            GroupBox {
                id: privacyFilterOptions
                title: "Privacy Filter Options"
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 20

                Column {
                    RadioButton {
                        text: "No Filter"
                        checked: true
                        exclusiveGroup: privacyFilter
                    }
                    RadioButton {
                        text: "Replace person by background"
                        exclusiveGroup: privacyFilter
                        onCheckedChanged: {
                            viewerWindow.colorFilter = checked;
                        }
                    }
                    RadioButton {
                        text: "Blur"
                        exclusiveGroup: privacyFilter
                    }
                }
            }

        }

        // Input
        Keys.onReturnPressed: {
            console.log(instanceViewer.width + " " + instanceViewer.height)
        }

        Keys.onPressed: {

            event.accepted = true;

            if (event.key === Qt.Key_M) {
                settings.visible = !settings.visible;
            }
            else if (event.key === Qt.Key_Escape) {
                Qt.quit();
            }
            else if (event.key === Qt.Key_Up) {
                if (checkBoxXYRot.checked === true) {
                    instanceViewer.rotateAxisX(2)
                } else if (checkBoxZRot.checked === true) {
                    instanceViewer.rotateAxisX(-2)
                }
            }
            else if (event.key === Qt.Key_Down) {
                if (checkBoxXYRot.checked === true) {
                    instanceViewer.rotateAxisX(-2)
                } else if (checkBoxZRot.checked === true) {
                    instanceViewer.rotateAxisX(+2)
                }
            }
            else if (event.key === Qt.Key_Right) {
                if (checkBoxXYRot.checked === true) {
                    instanceViewer.rotateAxisY(-2)
                } else if (checkBoxZRot.checked === true) {
                    instanceViewer.rotateAxisZ(-2)
                }
            }
            else if (event.key === Qt.Key_Left) {
                if (checkBoxXYRot.checked === true) {
                    instanceViewer.rotateAxisY(2)
                } else if (checkBoxZRot.checked === true) {
                    instanceViewer.rotateAxisZ(2)
                }
            }
            else if (event.key === Qt.Key_W) {
                if (checkBoxYTrans.checked === false)
                    instanceViewer.translateAxisZ(-0.1)
                else
                    instanceViewer.translateAxisY(0.1)
            }
            else if (event.key === Qt.Key_S) {
                if (checkBoxYTrans.checked === false)
                    instanceViewer.translateAxisZ(0.1)
                else
                    instanceViewer.translateAxisY(-0.1)
            }
            else if (event.key === Qt.Key_A) {
                instanceViewer.translateAxisX(-0.1)
            }
            else if (event.key === Qt.Key_D) {
                instanceViewer.translateAxisX(0.1)
            }
            else if (event.key === Qt.Key_R) {
                instanceViewer.resetPerspective()
            } else {
                event.accepted = false;
            }
        }
    } // Instance Viewer

} // Application Window
