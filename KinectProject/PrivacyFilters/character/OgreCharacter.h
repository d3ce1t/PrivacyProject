/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#ifndef OGRE_CHARACTER_H
#define OGRE_CHARACTER_H

#include "ogreengine.h"
#include "cameranodeobject.h"
#include "character/SinbadCharacterController.h"

#include <QtQuick/QQuickView>

class OgreCharacter : public QQuickView
{
    Q_OBJECT

public:

    Ogre::OverlayElement* mDepthPanel;

    explicit OgreCharacter(QWindow *parent = 0);
    ~OgreCharacter();
    void addTime(qint64 time_ms);
    
signals:
    void ogreInitialized();

public slots:
    void initializeOgre();
    void addContent();

protected:
    void createCamera(void);
    void createScene(void);
    void destroyScene(void);
    void setupDepthMaterial();
    
private:
    CameraNodeObject *m_cameraObject;
    OgreEngine *m_ogreEngine;

    Ogre::Root *m_root;
    Ogre::Camera* m_camera;
    Ogre::SceneManager *m_sceneManager;

    SinbadCharacterController* mChara;
    qint64 m_lastTime;
};

#endif // OGRE_CHARACTER_H
