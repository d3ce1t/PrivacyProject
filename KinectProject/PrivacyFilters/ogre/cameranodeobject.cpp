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


CameraNodeObject::CameraNodeObject(QObject *parent) :
    OgreCameraWrapper(parent),
    m_yaw(0),
    m_pitch(0),
    m_roll(0),
    m_position(0, 0, 0)
{
}

void CameraNodeObject::setCamera(Ogre::Camera* cam)
{
  m_camera = cam;
  m_node = Ogre::Root::getSingleton().getSceneManager("mySceneManager")->getRootSceneNode()->createChildSceneNode();
  m_node->attachObject(cam);
  m_position = m_node->getPosition();
  //m_node->setPosition(0, 14.9733, 2.13904);
  //setPitch(-6);
  qDebug() << "CamPos" << m_camera->getPosition().x << m_camera->getPosition().y << m_camera->getPosition().z;
  qDebug() << "NodePos" << m_position.x << m_position.y << m_position.z;
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

qreal CameraNodeObject::roll() const
{
    return m_roll;
}

qreal CameraNodeObject::x() const
{
   return m_position.x;
}

qreal CameraNodeObject::y() const
{
    return m_position.y;
}

qreal CameraNodeObject::z() const
{
    return m_position.z;
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

void CameraNodeObject::setRoll(qreal r)
{
    m_roll = r;
    updateRotation();
}

void CameraNodeObject::setX(qreal x)
{
    m_position.x = x;
    updatePosition();
}

void CameraNodeObject::setY(qreal y)
{
    m_position.y = y;
    updatePosition();
}

void CameraNodeObject::setZ(qreal z)
{
    m_position.z = z;
    updatePosition();
}

void CameraNodeObject::updateRotation()
{
    m_node->resetOrientation();
    m_node->yaw(Ogre::Radian(Ogre::Degree(m_yaw)));
    m_node->pitch(Ogre::Radian(Ogre::Degree(m_pitch)));
    m_node->roll(Ogre::Radian(Ogre::Degree(m_roll)));
}

void CameraNodeObject::updatePosition()
{
    m_node->resetOrientation();
    m_node->setPosition(m_position);
    qDebug() << "CamPos" << m_camera->getPosition().x << m_camera->getPosition().y << m_camera->getPosition().z;
    qDebug() << "NodePos" << m_position.x << m_position.y << m_position.z;
}
