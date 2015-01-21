#ifndef OGRESCENE_H
#define OGRESCENE_H

#include "types/DepthFrame.h"
#include "types/ColorFrame.h"
#include <QWindow>
#include <QElapsedTimer>
#include <QReadWriteLock>

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
public:
    OgreScene();
    virtual ~OgreScene();
    void initialise(int width = 640, int height = 480);
    void prepareData(const dai::QHashDataFrames frames);
    void render();
    void enableFilter(bool flag);
    uint texture() const;
    void resize(int width, int height);

protected:
    void setupResources();
    void createCamera();
    void createScene();
    void convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY) const;

private:
    void createOpenGLContext();
    void activateOgreContext();
    void doneOgreContext();
    void updateFBO(int width, int height);
    void updateViewport();

    // OpenGL
    QOpenGLContext*         m_qtContext;
    QOpenGLContext*         m_glContext;
    QSurface*               m_qtSurface;
    QWindow                 m_surface;

    // Ogre
    QString                 m_resources_cfg;
    QString                 m_plugins_cfg;
    Ogre::Root*             m_root;
    Ogre::RenderWindow*     m_ogreWindow;
    Ogre::Camera*           m_camera;
    Ogre::SceneManager*     m_sceneManager;
    Ogre::RenderTexture*    m_renderTarget;
    uint                    m_nativeTextureId;
    Ogre::Viewport*         m_viewport;

    // Character and other items
    CharacterController* m_chara;
    QReadWriteLock          m_lock;
    bool                    m_initialised;
    qint64                  m_lastTime;
    QElapsedTimer           m_timer;
    int                     m_userId;
};

#endif // OGRESCENE_H
