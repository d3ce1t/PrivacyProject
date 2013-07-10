#ifndef SKELETON_PAINTER_H
#define SKELETON_PAINTER_H

#include "Painter.h"
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QStandardItemModel>
#include <QTableView>
#include "../types/Skeleton.h"
#include "../types/SkeletonJoint.h"
#include "../types/Quaternion.h"

namespace dai {

class SkeletonPainter : public Painter
{
public:
    explicit SkeletonPainter(QOpenGLContext* context);
    virtual ~SkeletonPainter();
    void prepareData(DataFrame* frame);
    Skeleton& frame();

protected:
    void initialise();
    void render();

private:
    void loadModels();
    void prepareShaderProgram();
    void drawLimb(const dai::SkeletonJoint& joint1, const dai::SkeletonJoint& joint2);
    void drawJoint(const dai::SkeletonJoint& joint, const QVector3D& color);
    float colorIntensity(float value);

    Skeleton*               m_frame;
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
    Quaternion              m_lastQuaternions[20];
};

} // End Namespace

#endif // SKELETON_PAINTER_H
