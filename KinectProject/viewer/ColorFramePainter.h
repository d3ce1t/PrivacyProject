#ifndef COLOR_FRAME_PAINTER_H
#define COLOR_FRAME_PAINTER_H

#include "Painter.h"
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include "types/ColorFrame.h"

class QOpenGLShaderProgram;

namespace dai {

class ColorFramePainter : public Painter
{
public:
    ColorFramePainter(QOpenGLContext* context);
    virtual ~ColorFramePainter();
    void prepareData(DataFrame* frame);
    ColorFrame& frame();

protected:
    void initialise();
    void render();

private:
    void prepareShaderProgram();
    void prepareVertexBuffer();
    void loadVideoTexture(void *texture, GLsizei width, GLsizei height, GLuint glTextureId);

    ColorFrame*              m_frame;
    const int                textureUnit;
    QOpenGLShaderProgram*    m_shaderProgram;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer            m_positionsBuffer;
    QOpenGLBuffer            m_texCoordBuffer;

    // OpenGL identifiers
    GLuint                   m_frameTexture; // Texture Object
    GLuint                   m_perspectiveMatrix; // Matrix in the shader
    GLuint                   m_posAttr; // Pos attr in the shader
    GLuint                   m_texCoord; // Texture coord in the shader
    GLuint                   m_texSampler; // Texture Sampler in the shader
    GLuint                   m_sampler; // Sampler
};

} // End Namespace

#endif // COLOR_FRAME_PAINTER_H
