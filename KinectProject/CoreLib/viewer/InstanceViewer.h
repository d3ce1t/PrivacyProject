#ifndef INSTANCE_VIEWER_H
#define INSTANCE_VIEWER_H

#include <QQuickItem>
#include <QMatrix4x4>
#include <QMutex>
#include <QHash>
#include <QList>
#include <QListWidget>
#include "types/DataFrame.h"
#include "types/UserFrame.h"
#include "types/FrameFilter.h"
#include "viewer/ScenePainter.h"
#include "viewer/QMLEnumsWrapper.h"

using namespace dai;

namespace dai {
    enum ViewerMode {
        MODE_2D,
        MODE_3D
    };
}

class QListWidget;

class InstanceViewer : public QQuickItem
{
    Q_OBJECT

public:
    explicit InstanceViewer();
    virtual ~InstanceViewer();
    void setMode(ViewerMode mode);
    ViewerMode getMode() const;

signals:
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
    void renderOpenGLScene();

signals:
    void frameRendered();

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    // Private Functions
    shared_ptr<DataFrame> applyFilter(shared_ptr<DataFrame> inputFrame, shared_ptr<UserFrame> userMask = nullptr) const;
    void testOutput();
    void updatePaintersMatrix();

    // Private member attributes
    QQuickWindow*                                     m_window;
    shared_ptr<ScenePainter>                          m_scene;
    ViewerMode                                        m_mode;
    bool                                              m_running;
    QMultiHash<DataFrame::FrameType, shared_ptr<FrameFilter>> m_filters;
};

#endif // INSTANCE_VIEWER_H
