#ifndef VIEWERENGINE_H
#define VIEWERENGINE_H

#include <QObject>
#include <QQuickWindow>
#include "viewer/ScenePainter.h"
#include "types/UserFrame.h"
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
    ViewerEngine();
    ~ViewerEngine();
    void startEngine(QQuickWindow *window);
    void setMode(dai::ViewerMode mode);
    dai::ViewerMode getMode() const;
    bool running() const;
    shared_ptr<dai::ScenePainter> scene();
    void renderOpenGLScene();

signals:
    void frameRendered();
    void plusKeyPressed();
    void minusKeyPressed();
    void spaceKeyPressed();

public slots:
    void onPlusKeyPressed();
    void onMinusKeyPressed();
    void onSpaceKeyPressed();
    void onNewFrame(QHashDataFrames dataFrames);
    void enableFilter(int filter);
    void resetPerspective();
    void rotateAxisX(float angle);
    void rotateAxisY(float angle);
    void rotateAxisZ(float angle);
    void translateAxisX(float value);
    void translateAxisY(float value);
    void translateAxisZ(float value);

protected:
    void setQuickWindow(QQuickWindow *window);

private:
    // Private Functions
    shared_ptr<DataFrame> applyFilter(shared_ptr<DataFrame> inputFrame, shared_ptr<UserFrame> userMask = nullptr) const;
    void testOutput();
    void updatePaintersMatrix();

    // Private member attributes
    QQuickWindow*                                             m_quickWindow;
    shared_ptr<ScenePainter>                                  m_scene;
    ViewerMode                                                m_mode;
    bool                                                      m_running;
    QMultiHash<DataFrame::FrameType, shared_ptr<FrameFilter>> m_filters;
};

#endif // VIEWERENGINE_H
