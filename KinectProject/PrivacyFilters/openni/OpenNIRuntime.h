#ifndef OPENNI_RUNTIME_H
#define OPENNI_RUNTIME_H

#include <NiTE.h>
#include <OpenNI.h>
#include <QMutex>
#include "types/SkeletonFrame.h"

namespace dai {

#define MAX_USERS 10

// Class Declaration
class OpenNIRuntime //: public nite::UserTracker::NewFrameListener
{
public:
    static OpenNIRuntime* getInstance();
    virtual ~OpenNIRuntime();
    void releaseInstance();
    openni::VideoFrameRef readColorFrame();
    openni::VideoFrameRef readDepthFrame();
    nite::UserTrackerFrameRef readUserTrackerFrame();
    openni::PlaybackControl* playbackControl();
    void convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY) const;
    void convertRealWorldCoordinatesToDepth(float x, float y, float z, float* pOutX, float* pOutY) const;
    nite::UserTracker& getUserTracker();
    openni::VideoStream& getDepthStream();
    void copySkeleton(const nite::Skeleton& srcSkeleton, dai::Skeleton& dstSkeleton);

private:
    static SkeletonJoint::JointType staticMap[15];

    static QMutex          mutex;
    static OpenNIRuntime* _instance;
    static int            _instance_counter;

    OpenNIRuntime();
    void initOpenNI();
    void shutdownOpenNI();

    // OpenNI Data
    openni::Device            m_device;
    openni::VideoStream       m_oniDepthStream;
    openni::VideoStream       m_oniColorStream;
    openni::VideoStream**     m_depthStreams;
    openni::VideoStream**	  m_colorStreams;
    nite::UserTracker         m_oniUserTracker;
};

} // End namespace

#endif // OPENNI_RUNTIME_H
