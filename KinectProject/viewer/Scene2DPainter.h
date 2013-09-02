#ifndef SCENE2DPAINTER_H
#define SCENE2DPAINTER_H

#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include "viewer/ScenePainter.h"
#include "types/UserFrame.h"
#include "QMLEnumsWrapper.h"

namespace dai {

class Scene2DPainter : public ScenePainter
{
public:
    Scene2DPainter();
    ~Scene2DPainter();
    void setMask(shared_ptr<UserFrame> mask);
    void setMask2(shared_ptr<UserFrame> mask);
    void enableFilter(QMLEnumsWrapper::ColorFilter type);

protected:
    void initialise() override;
    void render() override;

private:
    void createFrameBuffer();
    void enableBGRendering();
    void renderBackground();
    void enableFilterRendering();
    void renderFilter();
    void displayRenderedTexture();
    void prepareShaderProgram();
    void prepareVertexBuffer();
    void loadVideoTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);
    void loadMaskTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);


    QMLEnumsWrapper::ColorFilter m_currentFilter = QMLEnumsWrapper::FILTER_DISABLED;
    QOpenGLShaderProgram*   m_shaderProgram;
    shared_ptr<UserFrame>   m_mask;

    // OpenGL Buffer
    shared_ptr<QOpenGLFramebufferObject>  m_fbo;
    shared_ptr<QOpenGLFramebufferObject>  m_fboFilter;
    QOpenGLVertexArrayObject              m_vao;
    QOpenGLBuffer                         m_positionsBuffer;
    QOpenGLBuffer                         m_texCoordBuffer;

    // OpenGL identifiers
    GLuint                   m_bgTextureId;
    GLuint                   m_fgTextureId;
    GLuint                   m_maskTextureId;
    GLuint                   m_mask2TextureId;

    // Shader identifiers
    GLuint                   m_perspectiveMatrixUniform;
    GLuint                   m_posAttr;
    GLuint                   m_textCoordAttr;
    GLuint                   m_stageUniform;
    GLuint                   m_currentFilterUniform;
    GLuint                   m_texColorSampler;
    GLuint                   m_texMaskSampler;
    GLuint                   m_texBackgroundSampler;
    GLuint                   m_texMask2Sampler;
};

} // End Namespace

#endif // SCENE2DPAINTER_H
