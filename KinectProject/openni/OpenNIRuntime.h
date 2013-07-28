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
    void onNewFrame(nite::UserTracker& userTracker);

private:
    static SkeletonJoint::JointType staticMap[15];

    void oniLoadSkeleton();

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
    openni::VideoFrameRef	  m_oniColorFrame;
    nite::UserTrackerFrameRef m_oniUserTrackerFrame;
    QReadWriteLock            m_lockColor;
    QReadWriteLock            m_lockDepth;
};

} // End namespace

#endif // OPENNI_RUNTIME_H
