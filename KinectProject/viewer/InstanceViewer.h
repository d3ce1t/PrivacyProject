#ifndef VIEWER_H
#define VIEWER_H

#include <QQuickView>
#include <QList>
#include <QElapsedTimer>
#include <QTimer>
#include <QMatrix4x4>
#include "DepthFramePainter.h"
#include "SkeletonPainter.h"


namespace dai {
    class DataInstance;
    class ViewerPainter;
}

class InstanceViewer : private QQuickView
{
    Q_OBJECT

    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)

public:
    explicit InstanceViewer( QWindow* parent = 0 );
    virtual ~InstanceViewer();
    void initialise();
    float getFPS() {return m_fps;}
    dai::DepthFramePainter &getDepthPainter() const;
    dai::SkeletonPainter& getSkeletonPainter() const;
    void show();
    void play(dai::DataInstance* instance);
    void stop();

signals:
    void changeOfStatus();

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
    void playNextFrame();

protected:
    void resizeEvent(QResizeEvent* event);
    bool event(QEvent * ev);

private:
    // Private Functions
    void updatePaintersMatrix();

    // Private member attributes
    QList<dai::ViewerPainter*>  m_painters;
    QElapsedTimer               m_time;
    QTimer                      m_timer;
    QMatrix4x4                  matrix;
    dai::DataInstance*          m_instance;
    qint64                      m_lastTime;
    long long                   m_frames;
    float                       m_fps;
    bool                        m_running;
    bool                        m_initialised;
};

#endif // VIEWER_H
