#ifndef VIEWERENGINE_H
#define VIEWERENGINE_H

#include <QObject>
#include <QQuickWindow>
#include "viewer/Scene3DPainter.h"
#include "types/MaskFrame.h"
#include "types/ColorFrame.h"
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

namespace dai {

    enum ViewerMode {
        MODE_2D,
        MODE_3D
    };
}

using namespace dai;

class InstanceViewer;

class ViewerEngine : public QObject, public QOpenGLFunctions
{
    Q_OBJECT

public:
    ViewerEngine(dai::ViewerMode mode = MODE_2D);
    ~ViewerEngine();
    void startEngine(QQuickWindow *window);
    bool running() const;
    shared_ptr<dai::ScenePainter> scene();
    void renderOpenGLScene(QOpenGLFramebufferObject *fbo);
    void setInstanceViewer(InstanceViewer* viewer);
    InstanceViewer* viewer();

signals:
    void plusKeyPressed();
    void minusKeyPressed();
    void spaceKeyPressed();

public slots:
    void onPlusKeyPressed();
    void onMinusKeyPressed();
    void onSpaceKeyPressed();
    void prepareScene(QHashDataFrames dataFrames);
    void enableFilter(int filter);
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
    void render2D(QOpenGLFramebufferObject* fboDisplay);

    QQuickWindow*            m_quickWindow;
    shared_ptr<Scene3DPainter> m_scene;
    QMutex                   m_dataLock;
    ViewerMode               m_mode;
    bool                     m_running;
    QSize                    m_size;
    bool                     m_initialised;
    shared_ptr<ColorFrame>   m_colorFrame;
    bool                     m_needLoading;
    QOpenGLShaderProgram*    m_shaderProgram;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer            m_positionsBuffer;
    QOpenGLBuffer            m_texCoordBuffer;
    QMatrix4x4               m_matrix;
    InstanceViewer*          m_viewer;

    // Shader identifiers
    GLuint                   m_colorTextureId;
    GLuint                   m_perspectiveMatrixUniform;
    GLuint                   m_posAttr;
    GLuint                   m_textCoordAttr;
    GLuint                   m_texColorFrameSampler;
};

#endif // VIEWERENGINE_H
