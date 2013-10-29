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
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom)
public:
    explicit CameraNodeObject(Ogre::Camera *cam, QObject *parent = 0);

    Ogre::SceneNode *sceneNode() const;
    qreal yaw() const;
    qreal pitch() const;
    qreal zoom() const;
    void setYaw(qreal y);
    void setPitch(qreal p);
    void setZoom(qreal z);

private:
    void updateRotation();

    Ogre::SceneNode *m_node;

    qreal m_yaw;
    qreal m_pitch;
    qreal m_zoom;
};

#endif // CAMERANODEOBJECT_H
