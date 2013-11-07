#ifndef OGRESCENE_H
#define OGRESCENE_H

#include "ogreitem.h"
#include "ogreengine.h"
#include "cameranodeobject.h"
#include "ogre/SinbadCharacterController.h"
#include <QObject>

class OgreScene : public QObject
{
    Q_OBJECT

public:
    OgreScene();
    CameraNodeObject* cameraNode();
    OgreEngine* engine();
    void initialiseOgre(QQuickWindow* quickWindow);
    void addTime(qint64 time_ms);

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
    SinbadCharacterController* m_chara;
    qint64 m_lastTime;
};

#endif // OGRESCENE_H
