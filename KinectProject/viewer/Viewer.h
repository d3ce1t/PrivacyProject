#ifndef WINDOW_H
#define WINDOW_H

#include <QQuickView>
#include <QTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QMatrix4x4>
#include <QVariantMap>
#include <QString>
#include "dataset/DataInstance.h"
#include "DepthFrame.h"

class DepthStreamScene;
class BasicUsageScene;
class Grill;
class HistogramScene;

class Viewer : public QQuickView
{
    Q_OBJECT

    Q_PROPERTY(int frameId READ getFrameId NOTIFY changeOfStatus)
    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)

public:
    explicit Viewer( QWindow* parent = 0 );
    virtual ~Viewer();
    void initialise();
    int  getFrameId() {return m_frames;}
    float getFPS() {return m_fps;}
    void play(dai::DataInstance* handler);

public slots:
    void resetPerspective();
    void rotateAxisX(float angle);
    void rotateAxisY(float angle);
    void rotateAxisZ(float angle);
    void translateAxisX(float value);
    void translateAxisY(float value);
    void translateAxisZ(float value);
    void renderOpenGLScene();
    void update();

signals:
    void changeOfStatus();

protected:
    void resizeEvent(QResizeEvent* event);
    bool event(QEvent * ev);

private:
    dai::DataInstance*      m_handler;
    DepthStreamScene*       m_depthScene;

    //BasicUsageScene*        m_scene;
    //Grill*                  m_grill;
    //HistogramScene*         m_histogram;
    QElapsedTimer           m_time;
    QMatrix4x4              matrix;

    // Settings Flags
    long long               m_frames;
    float                   m_fps;
    bool                    m_initialised;
    bool                    m_running;
    QTimer                  m_timer;
    qint64                  m_lastTime;
};

#endif // WINDOW_H
