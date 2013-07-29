#ifndef OPENNI_RUNTIME_H
#define OPENNI_RUNTIME_H

#include <NiTE.h>
#include <OpenNI.h>
#include <QMutex>
#include <QReadWriteLock>
#include "types/ColorFrame.h"
#include "types/DepthFrame.h"
#include "types/UserFrame.h"
#include "types/SkeletonFrame.h"

namespace dai {

#define MAX_USERS 10

// Class Declaration
class OpenNIRuntime : public openni::VideoStream::NewFrameListener, public nite::UserTracker::NewFrameListener
{
public:
    static OpenNIRuntime* getInstance();

    virtual ~OpenNIRuntime();
    void releaseInstance();
    DepthFrame readDepthFrame();
    ColorFrame readColorFrame();
    UserFrame readUserFrame();
    SkeletonFrame readSkeletonFrame();
    void onNewFrame(openni::VideoStream& stream);
    void onNewFrame(nite::UserTracker& oniUserTracker);
    void convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY);

private:
    static SkeletonJoint::JointType staticMap[15];

    void oniLoadSkeleton(nite::UserTracker &oniUserTracker, nite::UserTrackerFrameRef oniUserTrackerFrame);

    static QMutex          mutex;
    static OpenNIRuntime* _instance;
    static int            _instance_counter;

    OpenNIRuntime();
    void initOpenNI();
    void shutdownOpenNI();

    ColorFrame                m_colorFrame;
    DepthFrame                m_depthFrame;
    UserFrame                 m_userFrame;
    SkeletonFrame             m_skeletonFrame;

    // OpenNI Data
    openni::Device            m_device;
    openni::VideoStream       m_oniColorStream;
    nite::UserTracker         m_oniUserTracker;
    QReadWriteLock            m_lockColor;
    QReadWriteLock            m_lockDepth;
    bool                      m_trackingStarted[MAX_USERS];
};

} // End namespace

#endif // OPENNI_RUNTIME_H
