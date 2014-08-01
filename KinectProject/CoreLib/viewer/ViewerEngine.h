#ifndef VIEWERENGINE_H
#define VIEWERENGINE_H

#include <QObject>
#include <QQuickWindow>
#include <QMutex>
#include "types/ColorFrame.h"
#include "types/MetadataFrame.h"
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QFlags>

using namespace dai;

class InstanceViewer;

class ViewerEngine : public QObject, public QOpenGLFunctions
{
    Q_OBJECT

public:
    enum DrawPrimitive {
        /*Color       = 0x01,
        Depth       = 0x02,
        Skeleton    = 0x04,
        Mask        = 0x08,
        BoundingBox = 0x16*/
        Disabled    = 0x00,
        Mask        = 0x01,
        BoundingBox = 0x02
    };
    Q_DECLARE_FLAGS(DrawMode, DrawPrimitive)

    ViewerEngine();
    ~ViewerEngine();
    void startEngine(QQuickWindow *window);
    bool running() const;
    void prepareScene(QHashDataFrames dataFrames);
    void renderOpenGLScene(QOpenGLFramebufferObject *fbo);
    void setInstanceViewer(InstanceViewer* viewer);
    InstanceViewer* viewer();
    void setDrawMode(DrawMode mode);

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
    DrawMode                 m_showMode;

    // Shader identifiers
    GLuint                   m_colorTextureId;
    GLuint                   m_perspectiveMatrixUniform;
    GLuint                   m_noTextureUniform;
    GLuint                   m_posAttr;
    GLuint                   m_textCoordAttr;
    GLuint                   m_texColorFrameSampler;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(ViewerEngine::DrawMode)

#endif // VIEWERENGINE_H
