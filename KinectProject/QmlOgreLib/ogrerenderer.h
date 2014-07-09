#ifndef OGRERENDERER_H
#define OGRERENDERER_H

#include "ogreitem.h"
#include "ogreengine.h"
#include <QQuickFramebufferObject>
#include <QSGTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

class OgreRenderer : public QQuickFramebufferObject::Renderer, public QOpenGLFunctions
{
public:
    OgreRenderer(const OgreItem* viewer);
    void setOgreEngine(OgreEngine* engine);
    void setCamera(Ogre::Camera* camera);

protected:
    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;
    void render() override;
    void prepareShaderProgram();

private:
    QOpenGLShaderProgram* m_shaderProgram;
    OgreItem* m_viewer;
    OgreEngine* m_engine;
    Ogre::Viewport* m_viewport;
    Ogre::Camera* m_camera;
    Ogre::RenderTexture *m_renderTarget;
    Ogre::TexturePtr m_rttTexture;
    QSGTexture* m_texture;
    QOpenGLFramebufferObject* m_fbo;
    QSize m_size;

    // Shader identifiers
    GLuint                   m_perspectiveMatrixUniform;
    GLuint                   m_posAttr;
    GLuint                   m_textCoordAttr;
    GLuint                   m_texColorSampler;
};

#endif // OGRERENDERER_H
