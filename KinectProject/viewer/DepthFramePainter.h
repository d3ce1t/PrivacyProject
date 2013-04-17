#ifndef DEPTHSTREAMSCENE_H
#define DEPTHSTREAMSCENE_H

#include "ViewerPainter.h"
#include <OpenNI.h>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include "types/DepthFrame.h"


class QOpenGLShaderProgram;

namespace dai {

class DepthFramePainter : public ViewerPainter
{
public:
    DepthFramePainter(DataInstance* instance, InstanceViewer* parent = 0);
    virtual ~DepthFramePainter();
    bool prepareNext();
    void resize( float w, float h );

protected:
    void initialise();
    void render();

private:
    void prepareShaderProgram();

    QVector3D               colors[1];
    bool                    m_isFrameAvailable;
    DepthFrame              m_frame;
    QMap<float, float>      m_pDepthHist;
    QOpenGLShaderProgram*   m_shaderProgram;

    // OpenGL identifiers
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_posAttr; // Pos attr in the shader
    GLuint                  m_colorAttr; // Texture coord in the shader
    GLuint                  m_pointSize;
};

} // End Namespace

#endif // DEPTHSTREAMSCENE_H
