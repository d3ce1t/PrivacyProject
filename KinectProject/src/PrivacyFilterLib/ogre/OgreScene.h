#ifndef OGRESCENE_H
#define OGRESCENE_H

#include "types/DepthFrame.h"
#include "types/ColorFrame.h"
#include <QWindow>
#include <QElapsedTimer>
#include <QReadWriteLock>
#include <QMatrix4x4>
#include <OgreMatrix4.h>

class CharacterController;
class QOpenGLContext;

namespace Ogre {
    class Root;
    class RenderWindow;
    class Camera;
    class SceneManager;
    class RenderTexture;
    class Viewport;
}

class OgreScene
{
    int m_width;
    int m_height;

    // OpenGL
    QOpenGLContext*      m_qtContext;
    QOpenGLContext*      m_glContext;
    QSurface*            m_qtSurface;
    QWindow              m_surface;

    // Ogre
    QString              m_resources_cfg;
    QString              m_plugins_cfg;
    Ogre::Root*          m_root;
    Ogre::RenderWindow*  m_ogreWindow;
    Ogre::Camera*        m_camera;
    Ogre::SceneManager*  m_sceneManager;
    Ogre::RenderTexture* m_renderTarget;
    uint                 m_nativeTextureId;
    Ogre::Viewport*      m_viewport;

    // Character and other items
    CharacterController* m_chara;
    QReadWriteLock       m_lock;
    bool                 m_initialised;
    qint64               m_lastTime;
    QElapsedTimer        m_timer;
    int                  m_userId;
    Ogre::Matrix4        m_matrix;
    bool                 m_useGivenMatrix;

public:
    OgreScene();
    virtual ~OgreScene();
    void initialise(int width = 640, int height = 480);
    void resize(int width, int height);
    void prepareData(const dai::QHashDataFrames frames);
    void render();
    void enableFilter(bool flag);
    uint texture() const;
    void setMatrix(const QMatrix4x4& matrix);

private:
    void setupResources();
    void createCamera();
    void updateFBO(int width, int height);
    void updateViewport();
    void createScene();
    void convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY) const;
    void createOpenGLContext();
    void activateOgreContext();
    void doneOgreContext();
};

#endif // OGRESCENE_H
