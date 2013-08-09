#ifndef INSTANCE_VIEWER_H
#define INSTANCE_VIEWER_H

#include <QQuickItem>
#include <QMatrix4x4>
#include <QMutex>
#include <QHash>
#include <QList>
#include <QListWidget>
#include "types/DataFrame.h"
#include "viewer/Painter.h"
#include "viewer/DummyPainter.h"
#include "viewer/QMLEnumsWrapper.h"
#include "filters/InvisibilityFilter.h"
#include "filters/DilateUserFilter.h"
#include "filters/BlurFilter.h"

using namespace dai;

class QListWidget;

class InstanceViewer : public QQuickItem
{
    Q_OBJECT

public:
    explicit InstanceViewer();
    virtual ~InstanceViewer();

public slots:
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
    QList<dai::Painter*>                              m_painters;
    QHash<dai::DataFrame::FrameType, dai::Painter*>   m_paintersIndex;
    QMatrix4x4                                        m_matrix;
    bool                                              m_running;
    QMutex                                            m_mutex;
    QMultiHash<DataFrame::FrameType, shared_ptr<FrameFilter>> m_filters;
    //shared_ptr<dai::DummyPainter>                     m_dummyPainter;
};

#endif // INSTANCE_VIEWER_H
