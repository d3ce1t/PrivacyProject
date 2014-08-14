/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#include "ogreitem.h"
#include "ogrerenderer.h"

OgreItem::OgreItem(QQuickItem *parent)
    : QQuickFramebufferObject(parent)
    , m_timerID(0)
    , m_camera(0)
    , m_ogreEngineItem(0)
{
    setTextureFollowsItemSize(true);
    setSmooth(false);
    m_timerID = startTimer(16);
}

void OgreItem::timerEvent(QTimerEvent *evt)
{
    Q_UNUSED(evt);
    update();
}

void OgreItem::setCamera(QObject *camera)
{
    m_camera = qobject_cast<OgreCameraWrapper*>(camera);
}

void OgreItem::setOgreEngine(OgreEngine *ogreEngine)
{
    m_ogreEngineItem = ogreEngine;
}

QQuickFramebufferObject::Renderer* OgreItem::createRenderer() const
{
    OgreRenderer* renderer = new OgreRenderer(this);
    renderer->setOgreEngine(m_ogreEngineItem);
    renderer->setCamera(m_camera->camera());
    return renderer;
}
