#ifndef OPENNI_RUNTIME_H
#define OPENNI_RUNTIME_H

#include <NiTE.h>
#include <OpenNI.h>
#include <QMutex>
#include <QReadWriteLock>

namespace dai {

// Class Declaration
class OpenNIRuntime : public openni::VideoStream::NewFrameListener, public nite::UserTracker::NewFrameListener
{
public:
    static OpenNIRuntime* getInstance();

    virtual ~OpenNIRuntime();
    void releaseInstance();
    openni::VideoFrameRef readDepthFrame();
    openni::VideoFrameRef readColorFrame();
    nite::UserTrackerFrameRef readUserTrackerFrame();
    void onNewFrame(openni::VideoStream& stream);
    void onNewFrame(nite::UserTracker& userTracker);

private:
    static QMutex          mutex;
    static OpenNIRuntime* _instance;
    static int            _instance_counter;

    OpenNIRuntime();
    void initOpenNI();
    void shutdownOpenNI();

    openni::Device            m_device;
    openni::VideoStream       m_colorStream;
    nite::UserTracker         m_pUserTracker;
    openni::VideoFrameRef	  m_colorFrame;
    openni::VideoFrameRef     m_depthFrame;
    nite::UserTrackerFrameRef m_userTrackerFrame;
    QReadWriteLock            m_lockColor;
    QReadWriteLock            m_lockDepth;
};

} // End namespace

#endif // OPENNI_RUNTIME_H
