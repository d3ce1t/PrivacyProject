import QtQuick 2.0

Item {
    id: wrapper
    width: 640
    height: 480
    focus: true

    // Joints Layer
    Rectangle {
        id: jointsLayer
        color: Qt.rgba(0, 0, 0, 0)
        anchors.fill: parent
        visible: skeleton.state === 2? true : false

        Text {
            id: textJointHead
            text: "(" + Math.round(skeleton.jointHead.x) + ", " + Math.round(skeleton.jointHead.y) + ")"
            x: skeleton.jointHead.x
            y: skeleton.jointHead.y
            color: "white"
        }

        Text {
            id: textJointNeck
            text: "(" + Math.round(skeleton.jointNeck.x) + ", " + Math.round(skeleton.jointNeck.y) + ")"
            x: skeleton.jointNeck.x
            y: skeleton.jointNeck.y
            color: "white"
        }

        Text {
            id: textJointLeftShoulder
            text: "(" + Math.round(skeleton.jointLeftShoulder.x) + ", " + Math.round(skeleton.jointLeftShoulder.y) + ")"
            x: skeleton.jointLeftShoulder.x
            y: skeleton.jointLeftShoulder.y
            color: "white"
        }

        Text {
            id: textJointRightShoulder
            text: "(" + Math.round(skeleton.jointRightShoulder.x) + ", " + Math.round(skeleton.jointRightShoulder.y) + ")"
            x: skeleton.jointRightShoulder.x
            y: skeleton.jointRightShoulder.y
            color: "white"
        }

        Text {
            id: textJointLeftElbow
            text: "(" + Math.round(skeleton.jointLeftElbow.x) + ", " + Math.round(skeleton.jointLeftElbow.y) + ")"
            x: skeleton.jointLeftElbow.x
            y: skeleton.jointLeftElbow.y
            color: "white"
        }

        Text {
            id: textJointRightElbow
            text: "(" + Math.round(skeleton.jointRightElbow.x) + ", " + Math.round(skeleton.jointRightElbow.y) + ")"
            x: skeleton.jointRightElbow.x
            y: skeleton.jointRightElbow.y
            color: "white"
        }

        Text {
            id: textJointLeftHand
            text: "(" + Math.round(skeleton.jointLeftHand.x) + ", " + Math.round(skeleton.jointLeftHand.y) + ")"
            x: skeleton.jointLeftHand.x
            y: skeleton.jointLeftHand.y
            color: "white"
        }

        Text {
            id: textJointRightHand
            text: "(" + Math.round(skeleton.jointRightHand.x) + ", " + Math.round(skeleton.jointRightHand.y) + ")"
            x: skeleton.jointRightHand.x
            y: skeleton.jointRightHand.y
            color: "white"
        }

        Text {
            id: textJointTorse
            text: "(" + Math.round(skeleton.jointTorso.x) + ", " + Math.round(skeleton.jointTorso.y) + ")"
            x: skeleton.jointTorso.x
            y: skeleton.jointTorso.y
            color: "white"
        }

        Text {
            id: textJointLeftHip
            text: "(" + Math.round(skeleton.jointLeftHip.x) + ", " + Math.round(skeleton.jointLeftHip.y) + ")"
            x: skeleton.jointLeftHip.x
            y: skeleton.jointLeftHip.y
            color: "white"
        }

        Text {
            id: textJointRightLip
            text: "(" + Math.round(skeleton.jointRightLip.x) + ", " + Math.round(skeleton.jointRightLip.y) + ")"
            x: skeleton.jointRightLip.x
            y: skeleton.jointRightLip.y
            color: "white"
        }

        Text {
            id: textJointLeftKnee
            text: "(" + Math.round(skeleton.jointLeftKnee.x) + ", " + Math.round(skeleton.jointLeftKnee.y) + ")"
            x: skeleton.jointLeftKnee.x
            y: skeleton.jointLeftKnee.y
            color: "white"
        }

        Text {
            id: textJointRightKnee
            text: "(" + Math.round(skeleton.jointRightKnee.x) + ", " + Math.round(skeleton.jointRightKnee.y) + ")"
            x: skeleton.jointRightKnee.x
            y: skeleton.jointRightKnee.y
            color: "white"
        }

        Text {
            id: textJointLeftFoot
            text: "(" + Math.round(skeleton.jointLeftFoot.x) + ", " + Math.round(skeleton.jointLeftFoot.y) + ")"
            x: skeleton.jointLeftFoot.x
            y: skeleton.jointLeftFoot.y
            color: "white"
        }

        Text {
            id: textJointRightFoot
            text: "(" + Math.round(skeleton.jointRightFoot.x) + ", " + Math.round(skeleton.jointRightFoot.y) + ")"
            x: skeleton.jointRightFoot.x
            y: skeleton.jointRightFoot.y
            color: "white"
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
                anchors.right: parent.right
                label: "Enable Bounding Box drawing"
                value: appSettings1.drawBoundingBox
                onClick : {
                    appSettings1.drawBoundingBox = value
                }
            }

            Checkbox {
                anchors.right: parent.right
                label: "Enable Center of Mass drawing"
                value: appSettings1.drawCenterOfMass
                onClick : {
                    appSettings1.drawCenterOfMass = value
                }
            }

            Checkbox {
                anchors.right: parent.right
                label: "Enable Skeletal drawing"
                value: appSettings1.drawSkeleton
                onClick : {
                    appSettings1.drawSkeleton = value
                }
            }

            Checkbox {
                anchors.right: parent.right
                label: "Enable Background drawing"
                value: appSettings2.drawBackground
                onClick : {
                    appSettings2.drawBackground = value
                }
            }

            Checkbox {
                anchors.right: parent.right
                label: "Enable Histogram"
                value: winObject.drawHistogram
                onClick : {
                    winObject.drawHistogram = value
                }
            }

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

            Checkbox {
                id: checkBoxOM1
                anchors.right: parent.right
                label: "Enable Depth Overlay"
                value: appSettings2.overlayMode <= 1 ? true : false
                onClick : {
                    if (value === true && checkBoxOM2.value === true) {
                        appSettings2.overlayMode = 1;
                    }
                    else if (value === false && checkBoxOM2.value === true)
                        appSettings2.overlayMode = 2;
                    else {
                        appSettings2.overlayMode = 0;
                        value = true
                    }
                }
            }

            Checkbox {
                id: checkBoxOM2
                anchors.right: parent.right
                label: "Enable RGB Overlay"
                value: appSettings2.overlayMode >= 1 ? true : false
                onClick : {
                    if (value === true && checkBoxOM1.value === true) {
                        appSettings2.overlayMode = 1;
                    }
                    else if (value === false && checkBoxOM1.value === true)
                        appSettings2.overlayMode = 0;
                    else {
                        appSettings2.overlayMode = 2;
                        value = true
                    }
                }
            }
        }
    }

    // Draw Status Labels
    Rectangle {
        color: Qt.rgba(0, 0.7, 1, 0.7)
        radius: 10
        anchors.fill: textStatusLabel
        anchors.margins: -10
        visible: winObject.drawStatusLabel
    }

    Text {
        id: textStatusLabel
        text: winObject.statusLabel
        color: "black"
        wrapMode: Text.WordWrap
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20
        visible: winObject.drawStatusLabel
    }

    // Draw Frames Id
    Rectangle {
        color: Qt.rgba(0, 0.7, 1, 0.7)
        radius: 10
        anchors.fill: textFrameId
        anchors.margins: -10
        visible: winObject.drawFrameId
    }

    Text {
        id: textFrameId
        text: "Frames " + winObject.frameId + " (" + Math.round(winObject.fps) + " fps)"
        color: "black"
        wrapMode: Text.WordWrap
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        visible: winObject.drawFrameId
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
