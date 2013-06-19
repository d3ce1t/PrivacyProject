#ifndef VIEWER_H
#define VIEWER_H

#include <QQuickView>
#include <QList>
#include <QElapsedTimer>
#include <QTimer>
#include <QMatrix4x4>
#include <QMutex>
#include <QtGui/QOpenGLFunctions>
#include "../types/StreamInstance.h"
#include "ViewerPainter.h"

namespace dai {
    class DataFrame;
}

typedef QList<dai::DataFrame*> DataFrameList;


class InstanceViewer : public QQuickView, protected QOpenGLFunctions
{
    Q_OBJECT
    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)

public:
    explicit InstanceViewer( QWindow* parent = 0 );
    virtual ~InstanceViewer();
    float getFPS() {return m_fps;}
    void show();
    void play(dai::StreamInstance* instance, bool restartAll = false);
    void stop();

signals:
    void changeOfStatus();
    void viewerClose(InstanceViewer* viewer);
    void beforeDisplaying(DataFrameList frameList, InstanceViewer* viewer);

public slots:
    void resetPerspective();
    void rotateAxisX(float angle);
    void rotateAxisY(float angle);
    void rotateAxisZ(float angle);
    void translateAxisX(float value);
    void translateAxisY(float value);
    void translateAxisZ(float value);

private slots:
    void renderOpenGLScene();
    void renderLater();
    void playNextFrame();

protected:
    void resizeEvent(QResizeEvent* event);
    bool event(QEvent * event);

private:
    // Private Functions
    void updatePaintersMatrix();

    // Private member attributes
    QList<dai::ViewerPainter*>  m_painters;
    QElapsedTimer               m_time;
    QMatrix4x4                  matrix;
    qint64                      m_lastTime;
    long long                   m_frames;
    float                       m_fps;
    bool                        m_running;
    bool                        m_update_pending;
    QMutex                      m_mutex;
};

#endif // VIEWER_H
