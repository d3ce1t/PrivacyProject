#ifndef WINDOW_H
#define WINDOW_H

#include <QQuickView>
#include <QTime>
#include <QElapsedTimer>
#include <NiTE.h>
#include <OpenNI.h>
#include <QMatrix4x4>
#include <QVariantMap>
#include <QString>

class DepthStreamScene;
class BasicUsageScene;
class Grill;
class HistogramScene;

class Window : public QQuickView
{
    Q_OBJECT

    Q_PROPERTY(bool drawStatusLabel READ getDrawStatusLabelFlag WRITE setDrawStatusLabelFlag NOTIFY changeOfStatus)
    Q_PROPERTY(bool drawFrameId READ getDrawFrameIdFlag WRITE setDrawFrameIdFlag NOTIFY changeOfStatus)
    Q_PROPERTY(bool drawHistogram READ getDrawHistogramFlag WRITE setDrawHistogramFlag NOTIFY changeOfStatus)
    Q_PROPERTY(int frameId READ getFrameId NOTIFY changeOfStatus)
    Q_PROPERTY(QString statusLabel READ getStatuLabel NOTIFY changeOfStatus)
    Q_PROPERTY(float fps READ getFPS NOTIFY changeOfStatus)

public:
    explicit Window( QWindow* parent = 0 );
    virtual ~Window();
    void initialise();
    bool getDrawStatusLabelFlag() {return g_drawStatusLabel;}
    bool getDrawFrameIdFlag() {return g_drawFrameId;}
    bool getDrawHistogramFlag() {return g_drawHistogram;}
    int  getFrameId() {return m_frameId;}
    QString getStatuLabel() {return *m_statusLabel;}
    float getFPS() {return m_fps;}

public slots:
    void resetPerspective();
    void rotateAxisX(float angle);
    void rotateAxisY(float angle);
    void rotateAxisZ(float angle);
    void translateAxisX(float value);
    void translateAxisY(float value);
    void translateAxisZ(float value);
    void setDrawStatusLabelFlag(bool value);
    void setDrawFrameIdFlag(bool value);
    void setDrawHistogramFlag (bool value);
    void renderOpenGLScene();
    void update();

signals:
    void changeOfStatus();

protected:
    void resizeEvent(QResizeEvent* event);
    //void keyPressEvent(QKeyEvent* event);

private:
    void initOpenNI();
    void printMessage(const nite::UserData& user, uint64_t ts, const char *msg);
    void updateUserState(const nite::UserData& user, uint64_t ts);

    const int               g_poseTimeoutToExit;
    const static int        MAX_USERS = 10;
    char                    g_userStatusLabels[MAX_USERS][100];

    bool                    g_visibleUsers[MAX_USERS];
    nite::SkeletonState     g_skeletonStates[MAX_USERS];


    openni::Device			m_device;
    openni::VideoStream 	m_colorStream;
    nite::UserTracker       m_pUserTracker;
    openni::VideoFrameRef	m_colorFrame;

    DepthStreamScene*       m_depthScene;
    BasicUsageScene*        m_scene;
    Grill*                  m_grill;
    HistogramScene*         m_histogram;
    QElapsedTimer           m_time;
    QMatrix4x4              matrix;
    openni::VideoMode       videoMode;

    // Settings Flags
    bool                    g_drawStatusLabel;
    bool                    g_drawFrameId;
    bool                    g_drawHistogram;

    int                     m_frameId;
    float                   m_fps;
    long long               m_renderedFrames;

    QString*                m_statusLabel;
};

#endif // WINDOW_H
