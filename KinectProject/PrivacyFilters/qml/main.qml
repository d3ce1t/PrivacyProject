import QtQuick 2.0
import QtQuick.Controls 1.1
import InstanceViewer 1.0
import edu.dai.kinect 1.0

ApplicationWindow {
    id: mainWindow
    width: 640
    height:  505
    minimumWidth: width / 2
    minimumHeight: height / 2

    ExclusiveGroup {
        id: privacyFilter
    }

    SequentialAnimation {
        id: overlayTextHide
        PauseAnimation {duration:  800}
        PropertyAnimation {target: overlayText; property: "opacity"; to: 0; duration: 100}
    }

    menuBar: MenuBar {
        Menu {
            title: "&File"
            MenuItem {
                text: "Close"
                shortcut: "Ctrl+Q"
                onTriggered: mainWindow.close()
            }
        }
        Menu {
            title: "&Window"
            MenuItem {
                text: "Show Joints Positions"
                onTriggered: Window.showJointsWindow()
            }
            MenuItem {
                text: "Show Joints Distances"
                onTriggered: Window.showDistancesWindow()
            }
            MenuItem {
                text: "Show Quaternions"
                onTriggered: Window.showQuaternionsWindow()
            }
        }
    }

    // Draw FPS
    Rectangle {
        color: Qt.rgba(0, 0.7, 1, 0.5)
        radius: 10
        anchors.fill: textFrameId
        anchors.margins: -10
        z: 2
    }

    Text {
        id: textFrameId
        text: Math.round(Window.fps) + " fps"
        color: "black"
        font.pixelSize: 11
        wrapMode: Text.WordWrap
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20
        z: 2
    }

    InstanceViewer {
        id: instanceViewer
        viewerEngine: ViewerEngine
        focus: true
        anchors.fill: parent
        z: 0

        // Overlay Text
        Text {
            id: overlayText
            text: "Texto de prueba"
            opacity: 0
            font.pointSize: 25
            font.bold: false
            style: Text.Outline
            font.capitalization: Font.AllUppercase
            color: Qt.rgba(0, 1.0, 0.2, 1)
            styleColor: Qt.darker(color)
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height / 5
            width: paintedWidth
            height: paintedHeight
        }

        // Drop Area
        DropArea {
            id: dragTarget
            anchors.fill: instanceViewer

            onDropped: {
                Window.processListItem(drop.source)
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

        // Input
        Keys.onReturnPressed: {
            console.log(instanceViewer.width + " " + instanceViewer.height)
        }

        Keys.onPressed: {

            event.accepted = true;

            if (event.key === Qt.Key_Plus) {
                ViewerEngine.onPlusKeyPressed();
            }
            else if (event.key === Qt.Key_Minus) {
                ViewerEngine.onMinusKeyPressed();
            }
            else if (event.key === Qt.Key_Space) {
                ViewerEngine.onSpaceKeyPressed();
            }
            if (event.key === Qt.Key_M) {
                settings.visible = !settings.visible;
            }
            else if (event.key === Qt.Key_Escape) {
                Qt.quit();
            }
            else if (event.key === Qt.Key_Up) {
                if (checkBoxXYRot.checked === true) {
                    ViewerEngine.rotateAxisX(2)
                } else if (checkBoxZRot.checked === true) {
                    ViewerEngine.rotateAxisX(-2)
                }
            }
            else if (event.key === Qt.Key_Down) {
                if (checkBoxXYRot.checked === true) {
                    ViewerEngine.rotateAxisX(-2)
                } else if (checkBoxZRot.checked === true) {
                    ViewerEngine.rotateAxisX(+2)
                }
            }
            else if (event.key === Qt.Key_Right) {
                if (checkBoxXYRot.checked === true) {
                    ViewerEngine.rotateAxisY(-2)
                } else if (checkBoxZRot.checked === true) {
                    ViewerEngine.rotateAxisZ(-2)
                }
            }
            else if (event.key === Qt.Key_Left) {
                if (checkBoxXYRot.checked === true) {
                    ViewerEngine.rotateAxisY(2)
                } else if (checkBoxZRot.checked === true) {
                    ViewerEngine.rotateAxisZ(2)
                }
            }
            else if (event.key === Qt.Key_W) {
                if (checkBoxYTrans.checked === false)
                    ViewerEngine.translateAxisZ(-0.1)
                else
                    ViewerEngine.translateAxisY(0.1)
            }
            else if (event.key === Qt.Key_S) {
                if (checkBoxYTrans.checked === false)
                    ViewerEngine.translateAxisZ(0.1)
                else
                    ViewerEngine.translateAxisY(-0.1)
            }
            else if (event.key === Qt.Key_A) {
                ViewerEngine.translateAxisX(-0.1)
            }
            else if (event.key === Qt.Key_D) {
                ViewerEngine.translateAxisX(0.1)
            }
            else if (event.key === Qt.Key_R) {
                ViewerEngine.resetPerspective()
            } else {
                event.accepted = false;
            }
        }
    } // Instance Viewer

    // Settings Overlay
    Rectangle {
        id: settings
        color: Qt.rgba(0, 0.7, 1, 0.7)
        radius: 15
        anchors.fill: instanceViewer
        anchors.margins: 30
        visible: false
        z: 2
        focus: true

        ExclusiveGroup {
            id: rotationOptions
        }

        GroupBox {
            id: movingOptions
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

        GroupBox {
            title: "Camera Rotation"
            anchors.top: movingOptions.bottom
            anchors.left: parent.left
            anchors.margins: 20

            Column {
                spacing: 5

                Label {
                    text: "Yaw: " + sliderYaw.value
                }

                Slider {
                    id: sliderYaw
                    orientation: Qt.Horizontal
                    maximumValue: 180
                    minimumValue: -180
                    stepSize: 1
                    value: 0
                    onValueChanged: ogreitem.camera.yaw = value
                }

                Label {
                    text: "Pitch: " + sliderPitch.value
                }

                Slider {
                    id: sliderPitch
                    orientation: Qt.Horizontal
                    maximumValue: 180
                    minimumValue: -180
                    stepSize: 1
                    value: 0
                    onValueChanged: ogreitem.camera.pitch = value
                }

                Label {
                    text: "Roll: " + sliderRoll.value
                }

                Slider {
                    id: sliderRoll
                    orientation: Qt.Horizontal
                    maximumValue: 180
                    minimumValue: -180
                    stepSize: 1
                    value: 0
                    onValueChanged: ogreitem.camera.roll = value
                }
            }
        } // Camera Options

        GroupBox {
            title: "Camera Position"
            anchors.top: parent.top
            anchors.left: movingOptions.right
            anchors.margins: 20

            Column {
                spacing: 5

                Label {
                    text: "X: " + sliderCamX.value
                }

                Slider {
                    id: sliderCamX
                    orientation: Qt.Horizontal
                    maximumValue: 200
                    minimumValue: -200
                    value: 0
                    onValueChanged: ogreitem.camera.x = value
                }

                Label {
                    text: "Y: " + sliderCamY.value
                }

                Slider {
                    id: sliderCamY
                    orientation: Qt.Horizontal
                    maximumValue: 200
                    minimumValue: -200
                    value: 0
                    onValueChanged: ogreitem.camera.y = value
                }

                Label {
                    text: "Z: " + sliderCamZ.value
                }

                Slider {
                    id: sliderCamZ
                    orientation: Qt.Horizontal
                    maximumValue: 1000
                    minimumValue: -1000
                    value: 0
                    onValueChanged: ogreitem.camera.z = value
                }
            }
        } // Camera Position

    } // Settins Overlay
} // Application Window
