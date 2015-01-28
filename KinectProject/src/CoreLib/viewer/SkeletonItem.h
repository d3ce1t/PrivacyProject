#ifndef SKELETONITEM_H
#define SKELETONITEM_H

#include "viewer/SceneItem.h"
#include "types/SkeletonFrame.h"
#include <QOpenGLShaderProgram>

namespace dai {

class SkeletonItem : public SceneItem
{
    QOpenGLShaderProgram* m_shaderProgram;
    shared_ptr<SkeletonFrame> m_frame;
    bool m_mode3d;

    // OpenGL identifiers
    GLuint m_posAttr;
    GLuint m_colorAttr; // Texture coord in the shader
    GLuint m_perspectiveMatrix; // Matrix in the shader
    GLuint m_pointSizeUniform;
    GLuint m_unitsUniform;
    GLuint m_frameSizeUniform;
    GLuint m_mode3dUniform;

public:
    SkeletonItem();
    virtual ~SkeletonItem() = default;
    void setSkeleton(shared_ptr<SkeletonFrame> skeleton);
    void setMode3D(bool value);

protected:
    void initialise() override;
    void render(int pass) override;

private:
    static QVector3D staticJointsColor[20];
    void prepareShaderProgram();
    void drawLimb(const SkeletonJoint& joint1, const SkeletonJoint& joint2);
};

} // End Namespace

#endif // SKELETONITEM_H
