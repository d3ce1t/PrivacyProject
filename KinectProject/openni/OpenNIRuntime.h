#ifndef OPENNI_RUNTIME_H
#define OPENNI_RUNTIME_H

#include <NiTE.h>
#include <OpenNI.h>
#include <QMutex>

namespace dai {

// Forward declaration
class OpenNIListener;

// Class Declaration
class OpenNIRuntime
{
public:
    friend class OpenNIListener;

    static OpenNIRuntime* getInstance();

    virtual ~OpenNIRuntime();
    void releaseInstance();

    openni::VideoFrameRef readDepthFrame() const;
    openni::VideoFrameRef readColorFrame() const;
    nite::UserTrackerFrameRef readUserTrackerFrame() const;

private:
    static OpenNIRuntime* _instance;
    static int _instance_counter;

    OpenNIRuntime();
    void initOpenNI();

    openni::Device            m_device;
    openni::VideoStream       m_depthStream;
    openni::VideoStream       m_colorStream;
    nite::UserTracker         m_pUserTracker;
    openni::VideoFrameRef	  m_colorFrame;
    openni::VideoFrameRef     m_depthFrame;
    nite::UserTrackerFrameRef m_userTrackerFrame;
    OpenNIListener*           m_listener;
    static QMutex             mutex;

};

} // End namespace

#endif // OPENNI_RUNTIME_H
