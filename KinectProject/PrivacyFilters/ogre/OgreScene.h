#ifndef OGRESCENE_H
#define OGRESCENE_H

#include "playback/FrameListener.h"
#include "ogre/SinbadCharacterController.h"
#include "ogre/OgrePointCloud.h"
#include "types/DepthFrame.h"
#include "types/ColorFrame.h"
#include <QOpenGLContext>
#include <QWindow>
#include <QElapsedTimer>
#include <QReadWriteLock>

class OgreScene : public dai::FrameListener
{
public:
    OgreScene();
    virtual ~OgreScene();
    void initialise();
    void newFrames(const dai::QHashDataFrames dataFrames);
    void prepareData(const dai::QHashDataFrames frames);
    void render();
    void enableFilter(bool flag);

protected:
    void setupResources();
    void createPointCloud();
    void loadDepthData(shared_ptr<dai::DepthFrame> depthFrame);
    void loadColorData(shared_ptr<dai::ColorFrame> colorFrame);
    void createCamera();
    void createScene();
    void convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY) const;

private:
    void createOpenGLContext();
    void activateOgreContext();
    void doneOgreContext();

    Ogre::String            m_resources_cfg;
    Ogre::String            m_plugins_cfg;
    QOpenGLContext*         m_glContext;
    QOpenGLFunctions*       m_gles;
    QWindow                 m_surface;
    Ogre::Root*             m_root;
    Ogre::RenderWindow*     m_ogreWindow;
    Ogre::Camera*           m_camera;
    Ogre::SceneManager*     m_sceneManager;
    //Ogre::RenderTexture*    m_renderTarget;
    //Ogre::TexturePtr        m_rttTexture;
    Ogre::Viewport*         m_viewport;
    SinbadCharacterController* m_chara;
    QReadWriteLock          m_lock;
    dai::OgrePointCloud*    m_pointCloud;
    float*                  m_pDepthData;
    float*                  m_pColorData;
    int                     m_numPoints;
    bool                    m_initialised;

    qint64 m_lastTime;
    QElapsedTimer m_timer;
    int m_userId;
};

#endif // OGRESCENE_H
