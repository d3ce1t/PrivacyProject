import QtQuick 2.0

Item {
    id: wrapper
    width: 800
    height: 800
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
        }
    }

    // Draw Status Labels
    Rectangle {
        color: Qt.rgba(0, 0.7, 1, 0.7)
        radius: 10
        anchors.fill: textStatusLabel
        anchors.margins: -10
        visible: appSettings2.drawStatusLabel
    }

    Text {
        id: textStatusLabel
        text: appSettings2.statusLabel
        color: "black"
        wrapMode: Text.WordWrap
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20
        visible: appSettings2.drawStatusLabel
    }

    // Draw Frames Id
    Rectangle {
        color: Qt.rgba(0, 0.7, 1, 0.7)
        radius: 10
        anchors.fill: textFrameId
        anchors.margins: -10
        visible: appSettings2.drawFrameId
    }

    Text {
        id: textFrameId
        text: "Frames " + appSettings2.frameId + " (" + Math.round(appSettings2.fps) + " fps)"
        color: "black"
        wrapMode: Text.WordWrap
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 20
        visible: appSettings2.drawFrameId
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
            console.log("UP");
        }
        else if (event.key === Qt.Key_Down) {
            console.log("DOWN");
        }
        else if (event.key === Qt.Key_Right) {
            console.log("RIGHT");
        }
        else if (event.key === Qt.Key_Left) {
            console.log("LEFT");
        }
    }

    Keys.onReturnPressed: {
        console.log(skeleton.state)
        console.log("Width: " + wrapper.width + " Height: " + wrapper.height)
    }
}
