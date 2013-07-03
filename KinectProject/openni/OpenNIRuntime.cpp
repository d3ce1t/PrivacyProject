#include "OpenNIRuntime.h"
#include <iostream>
#include <cstdio>

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
    _instance_counter++;
    mutex.unlock();
    return _instance;
}

void OpenNIRuntime::releaseInstance()
{
    mutex.lock();
    _instance_counter--;

    if (_instance_counter == 0)
        delete this;
    mutex.unlock();
}

OpenNIRuntime::OpenNIRuntime()
{
    initOpenNI();
}

OpenNIRuntime::~OpenNIRuntime()
{
   shutdownOpenNI();
}

openni::VideoFrameRef OpenNIRuntime::readDepthFrame()
{
    QReadLocker locker(&m_lockDepth);
    return m_depthFrame;
}

openni::VideoFrameRef OpenNIRuntime::readColorFrame()
{
    QReadLocker locker(&m_lockColor);
    return m_colorFrame;
}

nite::UserTrackerFrameRef OpenNIRuntime::readUserTrackerFrame()
{
    QReadLocker locker(&m_lockDepth);
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

        /*const openni::SensorInfo* info = m_device.getSensorInfo(openni::SENSOR_COLOR);
        const openni::Array<openni::VideoMode>& videoModes = info->getSupportedVideoModes();

        std::cout << "Video Modes: " << videoModes.getSize() << std::endl;

        for (int i=0; i<videoModes.getSize(); ++i) {
            std::cout << i << ") " << videoModes[i].getResolutionX() << "x" << videoModes[i].getResolutionY() << std::endl;
        }*/

        if (m_colorStream.start() != openni::STATUS_OK)
            throw 5;

        if (m_pUserTracker.create(&m_device) != nite::STATUS_OK)
            throw 6;

        if (!m_pUserTracker.isValid() || !m_colorStream.isValid())
            throw 7;

        m_colorStream.addNewFrameListener(this);
        m_pUserTracker.addNewFrameListener(this);
    }
    catch (int ex)
    {
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        shutdownOpenNI();
        throw ex;
    }
}

void OpenNIRuntime::shutdownOpenNI()
{
    // Remove listeners
    m_pUserTracker.removeNewFrameListener(this);
    m_colorStream.removeNewFrameListener(this);

    // Release frame refs
    m_colorFrame.release();
    m_depthFrame.release();

    // Destroy streams and close device
    m_pUserTracker.destroy();
    m_colorStream.destroy();
    m_device.close();

    // Shutdown library
    nite::NiTE::shutdown();
    openni::OpenNI::shutdown();
}

void OpenNIRuntime::onNewFrame(openni::VideoStream& stream)
{
    QWriteLocker locker(&m_lockColor);
    openni::SensorType type = stream.getSensorInfo().getSensorType();

    if (type == openni::SENSOR_COLOR) {
        if (stream.readFrame(&m_colorFrame) != openni::STATUS_OK) {
            throw 1;
        }
    }
}

void OpenNIRuntime::onNewFrame(nite::UserTracker& userTracker)
{
    QWriteLocker locker(&m_lockDepth);
    if (userTracker.readFrame(&m_userTrackerFrame) != nite::STATUS_OK) {
        throw 2;
    }

    m_depthFrame = m_userTrackerFrame.getDepthFrame();
}

} // End namespace
