import QtQuick 2.0
import Ogre 1.0

Rectangle {
    id: ogre
    width: 1024
    height: 768
    color: "black"

    OgreItem {
        id: ogreitem
        anchors.fill: parent
        camera: Camera
        ogreEngine: OgreEngine

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
}
