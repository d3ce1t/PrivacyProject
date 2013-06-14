#include "OpenNIRuntime.h"
#include <iostream>
#include <cstdio>
#include "OpenNIListener.h"

namespace dai {

OpenNIRuntime* OpenNIRuntime::_instance = NULL;
int OpenNIRuntime::_instance_counter = 0;
QMutex             OpenNIRuntime::mutex;

OpenNIRuntime* OpenNIRuntime::getInstance()
{
    mutex.lock();
    if (_instance == NULL) {
        _instance = new OpenNIRuntime();
    }
    mutex.unlock();

    _instance_counter++;
    return _instance;
}

void OpenNIRuntime::releaseInstance()
{
    _instance_counter--;

    if (_instance_counter == 0)
        delete this;
}

OpenNIRuntime::OpenNIRuntime()
{
    m_listener = new OpenNIListener(this);
    initOpenNI();
}

OpenNIRuntime::~OpenNIRuntime()
{
    m_device.close();
    nite::NiTE::shutdown();
    openni::OpenNI::shutdown();
}

openni::VideoFrameRef OpenNIRuntime::readDepthFrame() const
{
    return m_depthFrame;
}

openni::VideoFrameRef OpenNIRuntime::readColorFrame() const
{
    return m_colorFrame;
}

nite::UserTrackerFrameRef OpenNIRuntime::readUserTrackerFrame() const
{
    return m_userTrackerFrame;
}

void OpenNIRuntime::initOpenNI()
{
    const char* deviceURI = openni::ANY_DEVICE;

    try {
        if (openni::OpenNI::initialize() != openni::STATUS_OK)
            throw 1;

        if (m_device.open(deviceURI) != openni::STATUS_OK)
            throw 2;

        if (nite::NiTE::initialize() != nite::STATUS_OK)
            throw 3;

        if (m_colorStream.create(m_device, openni::SENSOR_COLOR) != openni::STATUS_OK)
            throw 4;

        if (m_colorStream.start() != openni::STATUS_OK)
            throw 5;

        if (m_pUserTracker.create(&m_device) != nite::STATUS_OK)
            throw 7;

        if (!m_pUserTracker.isValid() || !m_colorStream.isValid())
            throw 8;

        /*if (m_pUserTracker.readFrame(&m_userTrackerFrame) != nite::STATUS_OK) {
            throw 9;
        }*/

        m_colorStream.addNewFrameListener(m_listener);
        m_pUserTracker.addNewFrameListener(m_listener);
    }
    catch (int ex)
    {
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        nite::NiTE::shutdown();
        openni::OpenNI::shutdown();
        throw ex;
    }
}

} // End namespace
