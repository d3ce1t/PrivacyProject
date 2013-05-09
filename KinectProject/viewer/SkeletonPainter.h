#ifndef BASICUSAGESCENE_H
#define BASICUSAGESCENE_H

#include "ViewerPainter.h"
#include <QMatrix4x4>
#include <QObject>
#include <QOpenGLBuffer>
#include "../types/Skeleton.h"
#include "../types/SkeletonJoint.h"
#include "../types/StreamInstance.h"
#include <QStandardItemModel>
#include <QTableView>

class QOpenGLShaderProgram;

namespace dai {

class SkeletonPainter : public ViewerPainter
{
public:
    explicit SkeletonPainter(StreamInstance* instance, InstanceViewer* parent = 0);
    virtual ~SkeletonPainter();
    bool prepareNext();
    void resize(float w, float h );

protected:
    void initialise();
    void render();

private:
    void loadModels();
    void prepareShaderProgram();
    void drawLimb(const dai::SkeletonJoint& joint1, const dai::SkeletonJoint& joint2);
    void drawJoint(const dai::SkeletonJoint& joint, const QVector3D& color);
    void drawQuaternions();
    float colorIntensity(float value);


    bool                    m_isFrameAvailable;
    Skeleton                m_skeleton;
    QOpenGLShaderProgram*   m_shaderProgram;
    GLuint                  m_posAttr;
    GLuint                  m_colorAttr; // Texture coord in the shader
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_pointSize;
    float                   m_width;
    float                   m_height;
    QStandardItemModel      m_joints_model;
    QStandardItemModel      m_distances_model;
    QStandardItemModel      m_quaternions_model;
    QTableView              m_joints_table_view;
    QTableView              m_distances_table_view;
    QTableView              m_quaternions_table_view;
};

} // End Namespace

#endif // BASICUSAGESCENE_H
