#ifndef INSTANCE_VIEWER_H
#define INSTANCE_VIEWER_H

#include <QQuickView>
#include <QMatrix4x4>
#include <QMutex>
#include <QtGui/QOpenGLFunctions>
#include "types/StreamInstance.h"
#include "types/DataFrame.h"
#include "viewer/Painter.h"
#include "viewer/PlaybackControl.h"
#include <QHash>
#include <QList>
#include <QListWidget>

class QListWidget;

class InstanceViewer : public QQuickView, protected QOpenGLFunctions, public dai::PlaybackControl::PlaybackListener
{
    Q_OBJECT
    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)

public:
    explicit InstanceViewer( QWindow* parent = 0 );
    virtual ~InstanceViewer();
    void show();
    void setPlayback(dai::PlaybackControl* playback);
    void onNewFrame(QList<dai::DataFrame*> dataFrames);

signals:
    void changeOfStatus();
    //void beforeDisplaying(dai::DataFrameList frameList, InstanceViewer* viewer);

public slots:
    void processListItem(QListWidget* widget);
    void resetPerspective();
    void rotateAxisX(float angle);
    void rotateAxisY(float angle);
    void rotateAxisZ(float angle);
    void translateAxisX(float value);
    void translateAxisY(float value);
    void translateAxisZ(float value);

private slots:
    void onNewFrameAux(QList<dai::DataFrame*> dataFrames);
    void renderOpenGLScene();

private:
    // Private Functions
    float getFPS() const;
    void updatePaintersMatrix();

    // Private member attributes
    qint64                                            m_token;
    float                                             m_fps;
    QHash<dai::DataFrame::FrameType, dai::Painter*>   m_painters;
    QMatrix4x4                                        m_matrix;
    bool                                              m_running;
    QMutex                                            m_mutex;
    dai::PlaybackControl*                             m_playback;
};

#endif // INSTANCE_VIEWER_H
