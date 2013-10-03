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
class OpenNIRuntime // : public nite::UserTracker::NewFrameListener
{
public:
    static OpenNIRuntime* getInstance();
    virtual ~OpenNIRuntime();
    void releaseInstance();
    void addNewDepthListener(openni::VideoStream::NewFrameListener* listener);
    void removeDepthListener(openni::VideoStream::NewFrameListener* listener);
    void addNewUserTrackerListener(nite::UserTracker::NewFrameListener* listener);
    void removeUserTrackerListener(nite::UserTracker::NewFrameListener* listener);
    void convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY);
    nite::UserTracker& getUserTracker();
    openni::VideoStream& getDepthStream();

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
    nite::UserTracker         m_oniUserTracker;
    SkeletonFrame             m_skeletonFrame;
    UserFrame                 m_userFrame;
    QReadWriteLock            m_lockUserTracker;
};

} // End namespace

#endif // OPENNI_RUNTIME_H
