#ifndef OGRESCENE_H
#define OGRESCENE_H

#include "ogre/SinbadCharacterController.h"
#include "ogre/OgrePointCloud.h"
#include "types/DepthFrame.h"
#include "types/ColorFrame.h"
#include <QWindow>
#include <QElapsedTimer>
#include <QReadWriteLock>
#include <QOpenGLFunctions>

class OgreScene : public QOpenGLFunctions
{
public:
    OgreScene();
    virtual ~OgreScene();
    void initialise();
    void prepareData(const dai::QHashDataFrames frames);
    void render();
    void enableFilter(bool flag);
    GLuint texture() const;

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

    // OpenGL
    QOpenGLContext*         m_qtContext;
    QOpenGLContext*         m_glContext;
    QSurface*               m_qtSurface;
    QWindow                 m_surface;

    // Ogre
    Ogre::String            m_resources_cfg;
    Ogre::String            m_plugins_cfg;
    Ogre::Root*             m_root;
    Ogre::RenderWindow*     m_ogreWindow;
    Ogre::Camera*           m_camera;
    Ogre::SceneManager*     m_sceneManager;
    Ogre::RenderTexture*    m_renderTarget;
    GLuint                  m_nativeTextureId;
    Ogre::Viewport*         m_viewport;

    // Character and other items
    SinbadCharacterController* m_chara;
    QReadWriteLock          m_lock;
    dai::OgrePointCloud*    m_pointCloud;
    float*                  m_pDepthData;
    float*                  m_pColorData;
    int                     m_numPoints;
    bool                    m_initialised;
    qint64                  m_lastTime;
    QElapsedTimer           m_timer;
    int                     m_userId;
};

#endif // OGRESCENE_H
