#ifndef BASICUSAGESCENE_H
#define BASICUSAGESCENE_H

#include "viewer/ViewerPainter.h"
#include <QMatrix4x4>
#include <QObject>
#include <QOpenGLBuffer>
#include "types/Skeleton.h"
#include "types/SkeletonJoint.h"


class QOpenGLShaderProgram;

namespace dai {

class SkeletonPainter : public ViewerPainter
{
public:
    explicit SkeletonPainter();
    virtual ~SkeletonPainter();
    void initialise();
    void render();
    void resize(float w, float h );
    void setFrame(const DataFrame& frame);

private:
    void prepareShaderProgram();
    void drawLimb(const dai::SkeletonJoint& joint1, const dai::SkeletonJoint& joint2);
    void drawJoint(const dai::SkeletonJoint& joint, const QVector3D& color);

    bool                    m_isFrameAvailable;
    Skeleton                m_skeleton;
    QOpenGLShaderProgram*   m_shaderProgram;
    GLuint                  m_posAttr;
    GLuint                  m_colorAttr; // Texture coord in the shader
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_pointSize;
    float                   m_width;
    float                   m_height;
};

} // End Namespace

#endif // BASICUSAGESCENE_H
