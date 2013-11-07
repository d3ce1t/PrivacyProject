/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#include "cameranodeobject.h"

#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>

#include <QDebug>

static const Ogre::Vector3 initialPosition(0, 0, 60);

CameraNodeObject::CameraNodeObject(QObject *parent) :
    OgreCameraWrapper(parent),
    m_yaw(0),
    m_pitch(0),
    m_zoom(1)
{

}

void CameraNodeObject::setCamera(Ogre::Camera* cam)
{
  m_camera = cam;
  m_node = Ogre::Root::getSingleton().getSceneManager("mySceneManager")->getRootSceneNode()->createChildSceneNode();
  m_node->attachObject(cam);
  cam->move(initialPosition);
}

Ogre::SceneNode* CameraNodeObject::sceneNode() const
{
    return m_node;
}

qreal CameraNodeObject::yaw() const
{
    return m_yaw;
}

qreal CameraNodeObject::pitch() const
{
    return m_pitch;
}

qreal CameraNodeObject::zoom() const
{
    return m_zoom;
}

void CameraNodeObject::setYaw(qreal y)
{
    m_yaw = y;
    updateRotation();
}

void CameraNodeObject::setPitch(qreal p)
{
    m_pitch = p;
    updateRotation();
}

void CameraNodeObject::setZoom(qreal z)
{
    m_zoom = z;
    m_node->resetOrientation();
    m_camera->setPosition(initialPosition * (1 / m_zoom));
    updateRotation();
}

void CameraNodeObject::updateRotation()
{
    m_node->resetOrientation();
    m_node->yaw(Ogre::Radian(Ogre::Degree(m_yaw)));
    m_node->pitch(Ogre::Radian(Ogre::Degree(m_pitch)));
}