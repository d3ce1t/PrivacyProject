#ifndef DEPTHSTREAMSCENE_H
#define DEPTHSTREAMSCENE_H

#include "ViewerPainter.h"
#include <OpenNI.h>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include "../types/DepthFrame.h"

#define MAX_DEPTH 10000

class QOpenGLShaderProgram;

namespace dai {

class ColorFramePainter
{
public:
    ColorFramePainter();
    virtual ~ColorFramePainter();
    void resize( float w, float h );
    void setFrame(const DataFrame &frame);

protected:
    void initialise();
    void render();

private:
    void prepareShaderProgram();
    void prepareFrameTexture(openni::RGB888Pixel* texture, const dai::DepthFrame &frame);
    void loadVideoTexture(openni::RGB888Pixel* texture, GLsizei width, GLsizei height, GLuint glTextureId);

    const int               textureUnit;
    QVector3D               colors[1];
    bool                    m_isFrameAvailable;

    float                   m_pDepthHist[MAX_DEPTH];
    openni::RGB888Pixel*	m_pTexMap;
    int                     m_pTexMapWidth;
    int                     m_pTexMapHeight;
    QOpenGLShaderProgram*   m_shaderProgram;

    // OpenGL identifiers
    GLuint                  m_frameTexture; // Texture Object
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_posAttr; // Pos attr in the shader
    GLuint                  m_texCoord; // Texture coord in the shader
    GLuint                  m_texSampler; // Texture Sampler in the shader
    GLuint                  m_sampler; // Sampler

    DepthFrame              m_frame;
};

} // End Namespace

#endif // DEPTHSTREAMSCENE_H
