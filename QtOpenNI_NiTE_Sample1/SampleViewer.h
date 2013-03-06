#ifndef SAMPLEVIEWER_H
#define SAMPLEVIEWER_H

#include <QtGui/QOpenGLShaderProgram>
#include "openglwindow.h"
#include <QElapsedTimer>
#include <NiTE.h>
#include <OpenNI.h>

#define MAX_DEPTH 10000

class SampleViewer : public OpenGLWindow
{
public:
    SampleViewer();
    virtual ~SampleViewer();
    void initialize();
    void render();

protected:
    virtual void keyPressEvent(QKeyEvent * ev);

private:
    void printMessage(const nite::UserData& user, uint64_t ts, const char *msg);
    void initOpenNI();
    void updateUserState(const nite::UserData& user, uint64_t ts);
    void DrawStatusLabel(nite::UserTracker* pUserTracker, const nite::UserData& user);
    void DrawFrameId(int frameId);
    void DrawCenterOfMass(nite::UserTracker* pUserTracker, const nite::UserData& user);
    void DrawBoundingBox(const nite::UserData& user);
    void DrawLimb(nite::UserTracker* pUserTracker, const nite::SkeletonJoint& joint1, const nite::SkeletonJoint& joint2, int color);
    void DrawSkeleton(nite::UserTracker* pUserTracker, const nite::UserData& userData);


    float                   m_pDepthHist[MAX_DEPTH];
    openni::RGB888Pixel*	m_pTexMap;
    unsigned int            m_nTexMapX;
    unsigned int            m_nTexMapY;
    openni::Device			m_device;
    nite::UserTracker*      m_pUserTracker;
    nite::UserId            m_poseUser;
    uint64_t                m_poseTime;
};

#endif // SAMPLEVIEWER_H
