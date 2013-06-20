#ifndef DEPTH_FRAME_PAINTER_H
#define DEPTH_FRAME_PAINTER_H

#include "ViewerPainter.h"
#include <OpenNI.h>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include "../types/DepthFrame.h"
#include "../types/StreamInstance.h"


class QOpenGLShaderProgram;

namespace dai {

class DepthFramePainter : public ViewerPainter
{
public:
    DepthFramePainter(StreamInstance* instance, InstanceViewer* parent = 0);
    virtual ~DepthFramePainter();
    bool prepareNext();
    DepthFrame& frame();

protected:
    void initialise();
    void render();

private:
    void prepareShaderProgram();

    const static QVector3D  m_colors[5];
    bool                    m_isFrameAvailable;
    DepthFrame*             m_frame;
    QMap<float, float>      m_pDepthHist;
    QOpenGLShaderProgram*   m_shaderProgram;

    // OpenGL identifiers
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_posAttr; // Pos attr in the shader
    GLuint                  m_colorAttr; // Texture coord in the shader
    GLuint                  m_pointSize;
};

} // End Namespace

#endif // DEPTH_FRAME_PAINTER_H
