#ifndef DEPTH_FRAME_PAINTER_H
#define DEPTH_FRAME_PAINTER_H

#include "Painter.h"
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include "../types/DepthFrame.h"
#include <QMutex>

namespace dai {

class DepthFramePainter : public Painter
{
public:
    DepthFramePainter(InstanceViewer* parent);
    virtual ~DepthFramePainter();
    void prepareData(shared_ptr<DataFrame> frame);
    DepthFrame& frame();

protected:
    void initialise();
    void render();

private:
    void convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY);
    void prepareShaderProgram();

    shared_ptr<DepthFrame>  m_frame;
    QMutex                  m_lockFrame;
    QMap<float, float>      m_pDepthHist;

    // OpenGL identifiers
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_posAttr; // Pos attr in the shader
    GLuint                  m_colorAttr; // Texture coord in the shader
    GLuint                  m_pointSize;
};

} // End Namespace

#endif // DEPTH_FRAME_PAINTER_H
