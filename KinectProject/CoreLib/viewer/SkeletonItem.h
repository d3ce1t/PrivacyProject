#ifndef SKELETONITEM_H
#define SKELETONITEM_H

#include "viewer/SceneItem.h"
#include "types/SkeletonFrame.h"
#include <QOpenGLShaderProgram>

namespace dai {

class SkeletonItem : public SceneItem
{
public:
    SkeletonItem();
    void setSkeleton(shared_ptr<SkeletonFrame> skeleton);

protected:
    void initialise() override;
    void render(int pass) override;

private:
    static QVector3D staticJointsColor[20];

    void prepareShaderProgram();
    void drawLimb(const SkeletonJoint& joint1, const SkeletonJoint& joint2);

    QOpenGLShaderProgram*    m_shaderProgram;
    shared_ptr<SkeletonFrame> m_frame;

    // OpenGL identifiers
    GLuint                  m_posAttr;
    GLuint                  m_colorAttr; // Texture coord in the shader
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_pointSize;
};

} // End Namespace

#endif // SKELETONITEM_H
