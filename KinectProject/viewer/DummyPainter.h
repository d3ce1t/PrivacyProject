#ifndef DUMMYPAINTER_H
#define DUMMYPAINTER_H

#include "Painter.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

namespace dai {

class DummyPainter : public Painter
{
public:
    DummyPainter(InstanceViewer* parent);
    virtual ~DummyPainter();
    void prepareData(shared_ptr<DataFrame> frame);

protected:
    void prepareShaderProgram();
    void prepareVertexBuffer();
    void initialise();
    void render();

private:
    void enableRenderToTexture();
    void renderToFrameBuffer();
    void createFrameBuffer();
    void createTexture(int width, int height, uint8_t* data);
    void loadTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);


    QOpenGLVertexArrayObject  m_vao;
    QOpenGLBuffer             m_positionsBuffer;
    QOpenGLBuffer             m_colorBuffer;
    QOpenGLBuffer             m_textureCoordBuffer;
    uint8_t*                  m_textureData;
    const int                 m_textureWidth;
    const int                 m_textureHeight;

    // OpenGL identifier
    GLuint                   m_posOffsetUniform;
    GLuint                   m_useTextureUniform;
    GLuint                   m_stageUniform;
    GLuint                   m_posAttr;
    GLuint                   m_colorAttr;
    GLuint                   m_textureCoordAttr;
    GLuint                   m_textureId; // Texture Object
    GLuint                   m_textureSampler; // Texture Sampler in the shader
    GLuint                   m_fboId;
    GLuint                   m_textureBG;
};

} // End Namespace

#endif // DUMMYPAINTER_H
