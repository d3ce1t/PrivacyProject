/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

import QtQuick 2.0
import Ogre 1.0

Rectangle {
    id: ogre
    width: 1024
    height: 768
    color: "black"

    Image {
        id: back
        anchors.fill: parent
        source: "qrc:/images/GrassandSky.png"
        Behavior on opacity { NumberAnimation { } }
    }


    OgreItem {
        id: ogreitem
        width: 600; height: 400
        anchors.left: toolbar1.left
        anchors.leftMargin: -5
        anchors.top: toolbar1.bottom
        anchors.topMargin: 6
        camera: Camera
        ogreEngine: OgreEngine

        Behavior on opacity { NumberAnimation { } }
        Behavior on width { NumberAnimation { } }
        Behavior on height { NumberAnimation { } }

        states: [
            State {
                name: "State1"

                PropertyChanges {
                    target: ogreitem
                    width: ogre.width
                    height: ogre.height
                }
                PropertyChanges {
                    target: toolbar1
                    x: 5
                    y: -toolbar1.height - 6
                }

                PropertyChanges {
                    target: toolbar4
                    anchors.top: ogreitem.top
                    anchors.topMargin: 5
                }
                PropertyChanges {
                    target: toolbar3
                    anchors.top: ogreitem.top
                    anchors.topMargin: 5
                }
                PropertyChanges {
                    target: back
                    opacity: 0
                }
            }
        ]

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
                        ogreitem.camera.zoom = Math.min(6, Math.max(0.1, ogreitem.camera.zoom - (mouse.y - prevY) / 100));
                    prevY = mouse.y
                }
            }
            onReleased: { prevX = -1; prevY = -1 }
        }
    }

    Rectangle {
        id: toolbar1
        x: 200
        y: 200
        width: 25
        height: 25
        radius: 5
        Behavior on x { NumberAnimation { } }
        Behavior on y { NumberAnimation { } }

        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#c83e3e3e"
            }

            GradientStop {
                position: 1
                color: "#c8919191"
            }
        }

        border.width: 2
        border.color: "#1a1a1a"

        Image {
            anchors.rightMargin: 5
            anchors.leftMargin: 5
            anchors.bottomMargin: 5
            anchors.topMargin: 5
            anchors.fill: parent
            smooth: true
            fillMode: "Stretch"
            source: "qrc:/images/move.gif"
        }

        MouseArea {
            anchors.fill: parent
            drag.target: toolbar1
            drag.axis: "XandYAxis"
            drag.minimumX: 0
            drag.minimumY: 0
            drag.maximumX: ogre.width - toolbar1.width
            drag.maximumY: ogre.height - toolbar1.height
        }
    }

    Rectangle {
        id: toolbar3
        width: 25
        height: 25
        radius: 5
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#c83e3e3e"
            }

            GradientStop {
                position: 1
                color: "#c8919191"
            }
        }
        anchors.top: toolbar1.top
        anchors.right: ogreitem.right
        anchors.rightMargin: 5
        border.color: "#1a1a1a"

        MouseArea {
            anchors.fill: parent
            onClicked: { ogreitem.state = ogreitem.state == '' ? 'State1' : '' }
        }

        Rectangle {
            id: toolbar31
            color: "#28ffffff"
            radius: 2
            border.width: 2
            border.color: "#000000"
            anchors.rightMargin: 7
            anchors.leftMargin: 7
            anchors.topMargin: 7
            anchors.bottomMargin: 7
            anchors.fill: parent

            Rectangle {
                id: toolbar311
                height: 3
                color: "#000000"
                anchors.right: parent.right
                anchors.left: parent.left
                anchors.top: parent.top
            }
        }
        border.width: 2
    }

    states: [
        State {
            name: "State1"

            PropertyChanges {
                target: rectangle1
                opacity: 0
            }
        }
    ]
}
