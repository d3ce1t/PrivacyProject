#ifndef INSTANCE_VIEWER_H
#define INSTANCE_VIEWER_H

#include <QQuickItem>
#include <QMatrix4x4>
#include <QMutex>
#include <QHash>
#include <QList>
#include <QListWidget>
#include "types/StreamInstance.h"
#include "types/DataFrame.h"
#include "viewer/Painter.h"
#include "viewer/PlaybackControl.h"


class QListWidget;
class InstanceViewerWindow;

class InstanceViewer : public QQuickItem
{
    Q_OBJECT

public:
    explicit InstanceViewer();
    virtual ~InstanceViewer();

public slots:
    void onNewFrame(QList<dai::DataFrame*> dataFrames);
    void resetPerspective();
    void rotateAxisX(float angle);
    void rotateAxisY(float angle);
    void rotateAxisZ(float angle);
    void translateAxisX(float value);
    void translateAxisY(float value);
    void translateAxisZ(float value);
    void renderOpenGLScene();
    //void cleanup();
    void sync();

signals:
    void frameRendered();

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    // Private Functions
    void updatePaintersMatrix();

    // Private member attributes
    QQuickWindow*                                     m_window;
    QHash<dai::DataFrame::FrameType, dai::Painter*>   m_painters;
    QMatrix4x4                                        m_matrix;
    bool                                              m_running;
    QMutex                                            m_mutex;
};

#endif // INSTANCE_VIEWER_H
