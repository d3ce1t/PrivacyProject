#ifndef VIEWERENGINE_H
#define VIEWERENGINE_H

#include <QObject>
#include <QQuickWindow>
#include "viewer/ScenePainter.h"
#include "types/MaskFrame.h"
#include "types/FrameFilter.h"

namespace dai {

    enum ViewerMode {
        MODE_2D,
        MODE_3D
    };
}

using namespace dai;

class ViewerEngine : public QObject
{
    Q_OBJECT

public:
    ViewerEngine(dai::ViewerMode mode = MODE_2D);
    ~ViewerEngine();
    void startEngine(QQuickWindow *window);
    bool running() const;
    shared_ptr<dai::ScenePainter> scene();
    void renderOpenGLScene(QOpenGLFramebufferObject *fbo);

signals:
    void frameRendered();
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
    // Private Functions
    //shared_ptr<DataFrame> applyFilter(shared_ptr<DataFrame> inputFrame, shared_ptr<MaskFrame> userMask = nullptr) const;
    void testOutput();
    void updatePaintersMatrix();

    // Private member attributes
    QQuickWindow*                                             m_quickWindow;
    shared_ptr<ScenePainter>                                  m_scene;
    ViewerMode                                                m_mode;
    bool                                                      m_running;
    //QMultiHash<DataFrame::FrameType, shared_ptr<FrameFilter>> m_filters;
    QSize                                                     m_size;
};

#endif // VIEWERENGINE_H
