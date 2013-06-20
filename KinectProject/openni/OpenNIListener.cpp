#include "OpenNIListener.h"

namespace dai {

OpenNIListener::OpenNIListener(OpenNIRuntime* openni)
{
    m_openni = openni;
}

OpenNIListener::~OpenNIListener()
{
    m_openni = NULL;
}

void OpenNIListener::onNewFrame(openni::VideoStream& stream)
{
    openni::SensorType type = stream.getSensorInfo().getSensorType();

    if (type == openni::SENSOR_COLOR && m_openni != NULL) {
        if (stream.readFrame(&(m_openni->m_colorFrame)) != openni::STATUS_OK) {
            throw 1;
        }
    }
}

void OpenNIListener::onNewFrame(nite::UserTracker& userTracker)
{
    if (userTracker.readFrame(&(m_openni->m_userTrackerFrame)) != nite::STATUS_OK) {
        throw 2;
    }

    if (m_openni != NULL)
        m_openni->m_depthFrame = m_openni->m_userTrackerFrame.getDepthFrame();
}

} // End namespace
