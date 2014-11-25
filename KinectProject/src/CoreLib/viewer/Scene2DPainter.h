#ifndef SCENE2DPAINTER_H
#define SCENE2DPAINTER_H

#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include "viewer/ScenePainter.h"
#include "types/MaskFrame.h"
#include "types.h"

namespace dai {

class Scene2DPainter : public ScenePainter
{
public:
    Scene2DPainter();
    ~Scene2DPainter();
    void setMask(shared_ptr<MaskFrame> mask);
    void setAvatarTexture(GLuint avatarTexture);
    void enableFilter(ColorFilter type);
    ColorFilter currentFilter() const;
    void resetPerspective() override;

protected:
    void setupTextures();
    void renderItems(QOpenGLFramebufferObject* target);
    void initialise() override;
    void render(QOpenGLFramebufferObject* target) override;

private:
    void createFrameBuffer();
    void extractBackground();
    void renderBackground();
    void renderComposite();
    void prepareShaderProgram();
    void prepareVertexBuffer();

    ColorFilter               m_currentFilter;
    QOpenGLShaderProgram*     m_shaderProgram;
    shared_ptr<MaskFrame>     m_mask;

    // OpenGL Buffer
    QOpenGLFramebufferObject* m_fboFirstPass; // render-to-texture (first-pass)
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
