#ifndef WINDOW_H
#define WINDOW_H

#include <QQuickView>
#include <QTime>
#include <QElapsedTimer>
#include <NiTE.h>
#include <OpenNI.h>
#include <QMatrix4x4>

class DepthStreamScene;
class BasicUsageScene;

class Window : public QQuickView
{
    Q_OBJECT
public:
    explicit Window( QWindow* parent = 0 );
    virtual ~Window();

public slots:
    void renderOpenGLScene();
    void update();

protected:
    void resizeEvent(QResizeEvent *event);

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
    nite::UserTracker*      m_pUserTracker;
    DepthStreamScene*       m_depthScene;
    BasicUsageScene*        m_scene;
    QElapsedTimer           m_time;
    QMatrix4x4              matrix;
    openni::VideoMode       videoMode;
};

#endif // WINDOW_H
