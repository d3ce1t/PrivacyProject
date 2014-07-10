#ifndef VIEWERENGINE_H
#define VIEWERENGINE_H

#include <QObject>
#include <QQuickWindow>
#include "types/ColorFrame.h"
#include "types/MetadataFrame.h"
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

using namespace dai;

class InstanceViewer;

class ViewerEngine : public QObject, public QOpenGLFunctions
{
    Q_OBJECT

public:
    ViewerEngine();
    ~ViewerEngine();
    void startEngine(QQuickWindow *window);
    bool running() const;
    void prepareScene(QHashDataFrames dataFrames);
    void renderOpenGLScene(QOpenGLFramebufferObject *fbo);
    void setInstanceViewer(InstanceViewer* viewer);
    InstanceViewer* viewer();

public slots:
    void resetPerspective();
    void rotateAxisX(float angle);
    void rotateAxisY(float angle);
    void rotateAxisZ(float angle);
    void translateAxisX(float value);
    void translateAxisY(float value);
    void translateAxisZ(float value);

private:
    void initialise();
    void prepareShaderProgram();
    void prepareVertexBuffer();
    void renderBoundingBoxes();

    QQuickWindow*            m_quickWindow;
    QMutex                   m_dataLock;
    bool                     m_running;
    bool                     m_initialised;
    shared_ptr<ColorFrame>   m_colorFrame;
    shared_ptr<MetadataFrame>m_metadataFrame;
    bool                     m_needLoading[2];
    QOpenGLShaderProgram*    m_shaderProgram;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer            m_positionsBuffer;
    QOpenGLBuffer            m_texCoordBuffer;
    QMatrix4x4               m_matrix;
    InstanceViewer*          m_viewer;

    // Shader identifiers
    GLuint                   m_colorTextureId;
    GLuint                   m_perspectiveMatrixUniform;
    GLuint                   m_noTextureUniform;
    GLuint                   m_posAttr;
    GLuint                   m_textCoordAttr;
    GLuint                   m_texColorFrameSampler;
};

#endif // VIEWERENGINE_H
