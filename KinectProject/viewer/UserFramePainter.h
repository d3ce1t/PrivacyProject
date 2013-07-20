#ifndef USER_FRAME_PAINTER_H
#define USER_FRAME_PAINTER_H

#include "Painter.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include "types/UserFrame.h"

class QOpenGLShaderProgram;

namespace dai {

class UserFramePainter : public Painter
{
public:
    UserFramePainter(InstanceViewer* parent);
    virtual ~UserFramePainter();
    void prepareData(shared_ptr<DataFrame> frame);
    UserFrame& frame();

protected:
    void initialise();
    void render();

private:
    void prepareShaderProgram();
    void prepareVertexBuffer();
    void loadVideoTexture(void *texture, GLsizei width, GLsizei height, GLuint glTextureId);

    shared_ptr<UserFrame>    m_frame;
    u_int8_t*                m_textureMask;
    const int                textureUnit;
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

#endif // USER_FRAME_PAINTER_H
