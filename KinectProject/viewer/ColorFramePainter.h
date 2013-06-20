#ifndef COLOR_FRAME_PAINTER_H
#define COLOR_FRAME_PAINTER_H

#include "ViewerPainter.h"
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include "../types/ColorFrame.h"

class QOpenGLShaderProgram;

namespace dai {

class ColorFramePainter : public ViewerPainter
{
public:
    ColorFramePainter(StreamInstance* instance, InstanceViewer* parent = 0);
    virtual ~ColorFramePainter();
    bool prepareNext();
    ColorFrame& frame();

protected:
    void initialise();
    void render();

private:
    void prepareShaderProgram();
    void loadVideoTexture(void *texture, GLsizei width, GLsizei height, GLuint glTextureId);

    const int               textureUnit;
    bool                    m_isFrameAvailable;
    QOpenGLShaderProgram*   m_shaderProgram;

    // OpenGL identifiers
    GLuint                  m_frameTexture; // Texture Object
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_posAttr; // Pos attr in the shader
    GLuint                  m_texCoord; // Texture coord in the shader
    GLuint                  m_texSampler; // Texture Sampler in the shader
    GLuint                  m_sampler; // Sampler

    ColorFrame*             m_frame;
};

} // End Namespace

#endif // COLOR_FRAME_PAINTER_H
