import QtQuick 2.0
import QtQuick.Controls 1.0
import InstanceViewer 1.0
import edu.dai.kinect 1.0
import Ogre 1.0

ApplicationWindow {
    id: mainWindow
    minimumWidth: 640
    minimumHeight: 505
    width: minimumWidth
    height:  minimumHeight

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
            title: "&Privacy Filters"
            MenuItem {
                text: "No Filter"
                checkable: true
                checked: true
                exclusiveGroup: privacyFilter
                shortcut: "Ctrl+0"
                onTriggered: {
                    overlayText.text = "No Filter Enabled"
                    overlayText.opacity = 1
                    overlayTextHide.start()
                    ViewerEngine.enableFilter(ColorFilter.FILTER_DISABLED) // GPU - No Filter
                }
            }

            MenuItem {
                text: "Invisibility"
                checkable: true
                exclusiveGroup: privacyFilter
                shortcut: "Ctrl+1"
                onTriggered: {
                    overlayText.text = "Invisibility Filter Enabled"
                    overlayText.opacity = 1
                    overlayTextHide.start()
                    ViewerEngine.enableFilter(ColorFilter.FILTER_INVISIBILITY) // GPU
                }
            }
            MenuItem {
                text: "Blur"
                checkable: true
                exclusiveGroup: privacyFilter
                shortcut: "Ctrl+2"
                onTriggered: {
                    overlayText.text = "Blur Filter Enabled"
                    overlayText.opacity = 1
                    overlayTextHide.start()
                    ViewerEngine.enableFilter(ColorFilter.FILTER_BLUR) // GPU
                }
            }
            MenuItem {
                text: "Pixelation"
                checkable: true
                exclusiveGroup: privacyFilter
                shortcut: "Ctrl+3"
                onTriggered: {
                    overlayText.text = "Pixelation Filter Enabled"
                    overlayText.opacity = 1
                    overlayTextHide.start()
                    ViewerEngine.enableFilter(ColorFilter.FILTER_PIXELATION) // GPU
                }
            }
            MenuItem {
                text: "Emboss"
                checkable: true
                exclusiveGroup: privacyFilter
                shortcut: "Ctrl+4"
                onTriggered: {
                    overlayText.text = "Emboss Filter Enabled"
                    overlayText.opacity = 1
                    overlayTextHide.start()
                    ViewerEngine.enableFilter(ColorFilter.FILTER_EMBOSS) // GPU
                }
            }
            MenuItem {
                text: "Silhouette"
                checkable: true
                exclusiveGroup: privacyFilter
                shortcut: "Ctrl+5"
                onTriggered: {
                    overlayText.text = "Silhouette Filter Enabled"
                    overlayText.opacity = 1
                    overlayTextHide.start()
                    ViewerEngine.enableFilter(ColorFilter.FILTER_SILHOUETTE) // GPU
                }
            }
            MenuItem {
                text: "Skeleton"
                checkable: true
                exclusiveGroup: privacyFilter
                shortcut: "Ctrl+6"
                onTriggered: {
                    overlayText.text = "Skeleton Filter Enabled"
                    overlayText.opacity = 1
                    overlayTextHide.start()
                    ViewerEngine.enableFilter(ColorFilter.FILTER_SKELETON) // GPU
                }
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

    InstanceViewer {
        id: instanceViewer
        viewerEngine: ViewerEngine
        implicitWidth: 640
        implicitHeight: 480
        //focus: true
        anchors.fill: parent
        z: 0

        // Draw FPS
        Rectangle {
            color: Qt.rgba(0, 0.7, 1, 0.5)
            radius: 10
            anchors.fill: textFrameId
            anchors.margins: -10
        }

        Text {
            id: textFrameId
            text: Math.round(Window.fps) + " fps"
            color: "black"
            font.pixelSize: 11
            wrapMode: Text.WordWrap
            anchors.left: instanceViewer.left
            anchors.top: instanceViewer.top
            anchors.margins: 20
        }

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

    OgreItem {
        id: ogreitem
        anchors.fill: parent
        camera: Camera
        ogreEngine: OgreEngine
        focus: true
        z: 1
        visible: false

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            property int prevX: -1
            property int prevY: -1

            onPositionChanged: {
                if (pressedButtons & Qt.LeftButton) {
                    if (prevX > -1)
                        ogreitem.camera.yaw -= (mouse.x - prevX) / 2
                    if (prevY > -1)
                        ogreitem.camera.pitch -= (mouse.y - prevY) / 2
                    prevX = mouse.x
                    prevY = mouse.y
                }
                if (pressedButtons & Qt.RightButton) {
                    if (prevY > -1)
                        ogreitem.camera.zoom = Math.min(12, Math.max(0.1, ogreitem.camera.zoom - (mouse.y - prevY) / 100));
                    prevY = mouse.y
                }
            }
            onReleased: { prevX = -1; prevY = -1 }
        }
    }


} // Application Window
