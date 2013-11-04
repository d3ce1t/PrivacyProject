#ifndef OGRESCENE_H
#define OGRESCENE_H

#include "ogreitem.h"
#include "ogreengine.h"
#include "cameranodeobject.h"
//#include "character/SinbadCharacterController.h"

class OgreScene
{
public:
    OgreScene();
    CameraNodeObject* cameraNode();
    OgreEngine* engine();
    void initialiseOgre(QQuickWindow* quickWindow);

protected:
    void createCamera(void);
    void createScene(void);
    void destroyScene(void);

private:
    QQuickWindow*           m_quickWindow;
    CameraNodeObject*       m_cameraObject;
    OgreEngine*             m_ogreEngine;
    Ogre::Root*             m_root;
    Ogre::Camera*           m_camera;
    Ogre::SceneManager*     m_sceneManager;

};

#endif // OGRESCENE_H
