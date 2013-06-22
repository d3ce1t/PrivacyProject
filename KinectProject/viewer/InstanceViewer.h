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
#include "Painter.h"
#include "types/DataFrame.h"
#include "viewer/PlaybackControl.h"

class InstanceViewer : public QQuickView, protected QOpenGLFunctions
{
    Q_OBJECT
    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)

public:
    explicit InstanceViewer( QWindow* parent = 0 );
    virtual ~InstanceViewer();
    void show();
    void addInstance(dai::StreamInstance* instance);
    void play(dai::StreamInstance* instance, bool restartAll = false);
    void stop();
    void setPlayback(dai::PlaybackControl* playback);

signals:
    void changeOfStatus();
    void viewerClose(InstanceViewer* viewer);
    void beforeDisplaying(dai::DataFrameList frameList, InstanceViewer* viewer);

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
    //void renderLater();
    void playNextFrame();

protected:
    bool event(QEvent * event);

private:
    // Private Functions
    float getFPS() const;
    void updatePaintersMatrix();

    // Private member attributes
    int                    m_token;
    float                  m_fps;
    QList<dai::Painter*>   m_painters;
    QMatrix4x4             matrix;
    bool                   m_running;
    QMutex                 m_mutex;
    dai::PlaybackControl*  m_playback;
};

#endif // VIEWER_H
