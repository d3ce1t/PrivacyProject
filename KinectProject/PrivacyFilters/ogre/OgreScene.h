#ifndef OGRESCENE_H
#define OGRESCENE_H

#include "ogreitem.h"
#include "ogreengine.h"
#include "cameranodeobject.h"
#include "ogre/SinbadCharacterController.h"
#include "ogre/OgrePointCloud.h"
#include <QObject>
#include "playback/PlaybackControl.h"
#include "playback/NodeListener.h"
#include "types/DepthFrame.h"
#include "types/ColorFrame.h"
#include <QReadWriteLock>

class OgreScene : public QObject, public dai::NodeListener
{
    Q_OBJECT

public:
    OgreScene();
    virtual ~OgreScene();
    CameraNodeObject* cameraNode();
    OgreEngine* engine();
    void initialiseOgre(QQuickWindow* quickWindow);
    void renderOgre();
    void newFrames(const dai::QHashDataFrames frames) override;

public slots:
    void enableFilter(bool flag);

protected:
    void createPointCloud();
    void loadDepthData(shared_ptr<dai::DepthFrame> depthFrame);
    void loadColorData(shared_ptr<dai::ColorFrame> colorFrame);
    void createCamera(void);
    void createScene(void);
    void destroyScene(void);
    void convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY) const;

private:
    QQuickWindow*           m_quickWindow;
    CameraNodeObject*       m_cameraObject;
    OgreEngine*             m_ogreEngine;
    Ogre::Root*             m_root;
    Ogre::Camera*           m_camera;
    Ogre::SceneManager*     m_sceneManager;
    SinbadCharacterController* m_chara;
    QReadWriteLock          m_lock;
    dai::OgrePointCloud*    m_pointCloud;
    float*                  m_pDepthData;
    float*                  m_pColorData;
    int                     m_numPoints;
    bool                    m_initialised;

    qint64 m_lastTime;
    QElapsedTimer   m_timer;
    int m_userId;
};

#endif // OGRESCENE_H
