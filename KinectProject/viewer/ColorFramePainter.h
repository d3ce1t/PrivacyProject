#ifndef COLOR_FRAME_PAINTER_H
#define COLOR_FRAME_PAINTER_H

#include "Painter.h"
#include <QOpenGLBuffer>
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
    void prepareData(shared_ptr<DataFrame> frame);
    ColorFrame& frame();

protected:
    void initialise();
    void render();

private:
    void prepareShaderProgram();
    void prepareVertexBuffer();
    void loadVideoTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);
    void loadMaskTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);

    shared_ptr<ColorFrame>   m_frame;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer            m_positionsBuffer;
    QOpenGLBuffer            m_texCoordBuffer;
    unique_ptr<u_int8_t>     m_textureData;

    // OpenGL identifiers    
    GLuint                   m_foregroundTexture; // Texture Object
    GLuint                   m_backgroundTexture; // Texture Object
    GLuint                   m_maskTexture; // Texture Object
    GLuint                   m_perspectiveMatrix; // Matrix in the shader
    GLuint                   m_posAttr; // Pos attr in the shader
    GLuint                   m_texCoord; // Texture coord in the shader
    GLuint                   m_texColorSampler; // Texture Sampler in the shader
    GLuint                   m_texMaskSampler; // Texture Sampler in the shader
    GLuint                   m_texBackgroundSampler; // Texture Sampler in the shader
};

} // End Namespace

#endif // COLOR_FRAME_PAINTER_H
