#ifndef SCENE2DPAINTER_H
#define SCENE2DPAINTER_H

#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include "viewer/ScenePainter.h"
#include "types/MaskFrame.h"
#include "QMLEnumsWrapper.h"

namespace dai {

class Scene2DPainter : public ScenePainter
{
public:
    Scene2DPainter();
    ~Scene2DPainter();
    void setMask(shared_ptr<MaskFrame> mask);
    void setAvatarTexture(GLuint avatarTexture);
    void enableFilter(QMLEnumsWrapper::ColorFilter type);
    QMLEnumsWrapper::ColorFilter currentFilter() const;
    void resetPerspective() override;

protected:
    void setupTextures();
    void renderItems();
    void initialise() override;
    void render(QOpenGLFramebufferObject* target) override;

private:
    void createFrameBuffer();
    void enableBGRendering();
    void extractBackground();
    void renderBackground();
    void displayRenderedTexture(QOpenGLFramebufferObject *target = nullptr);
    void renderComposite(QOpenGLFramebufferObject* target = nullptr);
    void prepareShaderProgram();
    void prepareVertexBuffer();

    QMLEnumsWrapper::ColorFilter m_currentFilter;
    QOpenGLShaderProgram*     m_shaderProgram;
    shared_ptr<MaskFrame>     m_mask;

    // OpenGL Buffer
    QOpenGLFramebufferObject* m_fboFirstPass; // render-to-texture (first-pass)
    QOpenGLFramebufferObject* m_fboSecondPass; // render-to-texture (second-pass)
    QOpenGLVertexArrayObject  m_vao;
    QOpenGLBuffer             m_positionsBuffer;
    QOpenGLBuffer             m_texCoordBuffer;

    // OpenGL identifiers
    GLuint                   m_bgTextureId;
    GLuint                   m_fgTextureId;
    GLuint                   m_maskTextureId;
    GLuint                   m_avatarTextureId;

    // Shader identifiers
    GLuint                   m_perspectiveMatrixUniform;
    GLuint                   m_posAttr;
    GLuint                   m_textCoordAttr;
    GLuint                   m_stageUniform;
    GLuint                   m_texColorSampler;
    GLuint                   m_texMaskSampler;
    GLuint                   m_texBackgroundSampler;
};

} // End Namespace

#endif // SCENE2DPAINTER_H
