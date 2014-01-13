/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#ifndef CAMERANODEOBJECT_H
#define CAMERANODEOBJECT_H

#include <QObject>

#include "ogrecamerawrapper.h"

namespace Ogre {
class SceneNode;
class Camera;
}

class CameraNodeObject :  public OgreCameraWrapper
{
    Q_OBJECT
    Q_PROPERTY(qreal yaw READ yaw WRITE setYaw)
    Q_PROPERTY(qreal pitch READ pitch WRITE setPitch)
    Q_PROPERTY(qreal roll READ roll WRITE setRoll)
    Q_PROPERTY(qreal x READ x WRITE setX)
    Q_PROPERTY(qreal y READ y WRITE setY)
    Q_PROPERTY(qreal z READ z WRITE setZ)
public:
    explicit CameraNodeObject(QObject *parent = 0);

    Ogre::SceneNode *sceneNode() const;
    qreal yaw() const;
    qreal pitch() const;
    qreal roll() const;
    qreal x() const;
    qreal y() const;
    qreal z() const;
    void setYaw(qreal y);
    void setPitch(qreal p);
    void setRoll(qreal r);
    void setX(qreal x);
    void setY(qreal y);
    void setZ(qreal z);
    void setCamera(Ogre::Camera* cam);

private:
    void updateRotation();
    void updatePosition();

    Ogre::SceneNode *m_node;

    qreal m_yaw;
    qreal m_pitch;
    qreal m_roll;
    Ogre::Vector3 m_position;
};

#endif // CAMERANODEOBJECT_H
