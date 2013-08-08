#ifndef SKELETON_FRAME_PAINTER_H
#define SKELETON_FRAME_PAINTER_H

#include "Painter.h"
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include "types/SkeletonFrame.h"
#include <QVector3D>

namespace dai {

class SkeletonFramePainter : public Painter
{
public:
    explicit SkeletonFramePainter(InstanceViewer* parent);
    void prepareData(shared_ptr<DataFrame> frame);

protected:
    void initialise();
    void render();

private:
    static QVector3D staticJointsColor[20];

    void prepareShaderProgram();
    void drawLimb(const SkeletonJoint& joint1, const SkeletonJoint& joint2);

    shared_ptr<SkeletonFrame> m_frame;

    // OpenGL identifiers
    GLuint                  m_posAttr;
    GLuint                  m_colorAttr; // Texture coord in the shader
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_pointSize;
};

} // End Namespace

#endif // SKELETON_FRAME_PAINTER_H
