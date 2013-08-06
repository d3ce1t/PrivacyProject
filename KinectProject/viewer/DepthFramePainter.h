#ifndef DEPTH_FRAME_PAINTER_H
#define DEPTH_FRAME_PAINTER_H

#include "Painter.h"
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include "types/DepthFrame.h"
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
    void prepareShaderProgram();
    void prepareVertexBuffer();

    shared_ptr<DepthFrame>  m_frame;
    QMutex                  m_lockFrame;

    // OpenGL Buffer
    QOpenGLVertexArrayObject              m_vao;
    QOpenGLBuffer                         m_positionsBuffer;
    QOpenGLBuffer                         m_distancesBuffer;

    // OpenGL identifiers
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_indexAttr; // Pos attr in the shader
    GLuint                  m_distanceAttr;
    GLuint                  m_widthUniform;
    GLuint                  m_heightUniform;

};

} // End Namespace

#endif // DEPTH_FRAME_PAINTER_H
