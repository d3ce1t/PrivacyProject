#ifndef DEPTHSTREAMSCENE_H
#define DEPTHSTREAMSCENE_H

#include "ViewerPainter.h"
#include <OpenNI.h>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include "types/DepthFrame.h"

#define MAX_DEPTH 10000

class QOpenGLShaderProgram;

namespace dai {

class DepthFramePainter : public ViewerPainter
{
public:
    DepthFramePainter();
    virtual ~DepthFramePainter();
    void initialise();
    void render();
    void resize( float w, float h );
    void setFrame(const DataFrame &frame);

private:
    void prepareShaderProgram();

    QVector3D               colors[1];
    bool                    m_isFrameAvailable;

    float                   m_pDepthHist[MAX_DEPTH];
    QOpenGLShaderProgram*   m_shaderProgram;

    // OpenGL identifiers
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_posAttr; // Pos attr in the shader
    GLuint                  m_colorAttr; // Texture coord in the shader
    GLuint                  m_pointSize;

    DepthFrame              m_frame;
};

} // End Namespace

#endif // DEPTHSTREAMSCENE_H
