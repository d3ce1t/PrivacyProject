#ifndef OPENNI_RUNTIME_H
#define OPENNI_RUNTIME_H

#include <NiTE.h>
#include <OpenNI.h>
#include <QReadWriteLock>
#include <QMutex>
#include <QWaitCondition>
#include "types/SkeletonFrame.h"
#include "types/UserFrame.h"

namespace dai {

#define MAX_USERS 10

// Class Declaration
class OpenNIRuntime : public nite::UserTracker::NewFrameListener
{
public:
    static OpenNIRuntime* getInstance();
    virtual ~OpenNIRuntime();
    void releaseInstance();
    SkeletonFrame readSkeletonFrame();
    UserFrame readUserFrame();
    void addNewColorListener(openni::VideoStream::NewFrameListener* listener);
    void addNewDepthListener(openni::VideoStream::NewFrameListener* listener);
    //void addNewUserTrackerListener(nite::UserTracker::NewFrameListener* listener);
    void removeColorListener(openni::VideoStream::NewFrameListener* listener);
    void removeDepthListener(openni::VideoStream::NewFrameListener* listener);
    //void removeUserTrackerListener(nite::UserTracker::NewFrameListener* listener);
    void convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY) const;
    void convertRealWorldCoordinatesToDepth(float x, float y, float z, float* pOutX, float* pOutY) const;
    void onNewFrame(nite::UserTracker& oniUserTracker);

private:
    static SkeletonJoint::JointType staticMap[15];

    // Hack: I cannot add two listener to the same userTracker so I use the listener from this class.
    // However, due to this hack I cannot have more than one instance of each instance type.
    void notifyNewUserFrame();
    void waitForNewUserFrame();
    void notifyNewSkeletonFrame();
    void waitForNewSkeletonFrame();

    void loadSkeleton(nite::UserTracker &oniUserTracker, nite::UserTrackerFrameRef& oniUserTrackerFrame);
    void loadUser(nite::UserTrackerFrameRef& oniUserTrackerFrame);

    static QMutex          mutex;
    static OpenNIRuntime* _instance;
    static int            _instance_counter;

    OpenNIRuntime();
    void initOpenNI();
    void shutdownOpenNI();

    // OpenNI Data
    openni::Device            m_device;
    openni::VideoStream       m_oniColorStream;
    openni::VideoStream       m_oniDepthStream;
    nite::UserTracker         m_oniUserTracker;
    SkeletonFrame             m_skeletonFrame;
    UserFrame                 m_userFrame;
    QReadWriteLock            m_lockUserTracker;

    // Hack for User Frame
    QMutex          m_lockUserSync;
    QWaitCondition  m_userSync;
    bool            m_newUserFrameGenerated;

    // Hack for SkeletonFrame
    QMutex          m_lockSkeletonSync;
    QWaitCondition  m_skeletonSync;
    bool            m_newSkeletonFrameGenerated;
};

} // End namespace

#endif // OPENNI_RUNTIME_H
