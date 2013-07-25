#ifndef SKELETON_PAINTER_H
#define SKELETON_PAINTER_H

#include "Painter.h"
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include "../types/Skeleton.h"
#include "../types/SkeletonJoint.h"
#include "../types/Quaternion.h"

namespace dai {

class SkeletonPainter : public Painter
{
public:
    explicit SkeletonPainter(InstanceViewer* parent);
    void prepareData(shared_ptr<DataFrame> frame);
    Skeleton& frame();

protected:
    void initialise();
    void render();

private:
    void prepareShaderProgram();
    void drawLimb(const dai::SkeletonJoint& joint1, const dai::SkeletonJoint& joint2);
    void drawJoint(const dai::SkeletonJoint& joint, const QVector3D& color);

    shared_ptr<Skeleton>    m_frame;

    // OpenGL identifiers
    GLuint                  m_posAttr;
    GLuint                  m_colorAttr; // Texture coord in the shader
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_pointSize;
};

} // End Namespace

#endif // SKELETON_PAINTER_H
