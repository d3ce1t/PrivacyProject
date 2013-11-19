#ifndef OGRESCENE_H
#define OGRESCENE_H

#include "ogreitem.h"
#include "ogreengine.h"
#include "cameranodeobject.h"
#include "ogre/SinbadCharacterController.h"
#include <QObject>
#include "playback/PlaybackListener.h"

class OgreScene : public QObject, public dai::PlaybackListener
{
    Q_OBJECT

public:
    OgreScene();
    CameraNodeObject* cameraNode();
    OgreEngine* engine();
    void initialiseOgre(QQuickWindow* quickWindow);

protected:
    void onNewFrame(const QHash<dai::DataFrame::FrameType, shared_ptr<dai::DataFrame>>& frames);
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
    QElapsedTimer   m_timer;
    int m_userId;
};

#endif // OGRESCENE_H
