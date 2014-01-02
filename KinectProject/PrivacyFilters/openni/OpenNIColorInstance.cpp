#include "OpenNIColorInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIColorInstance::OpenNIColorInstance()
{
    this->m_type = dai::INSTANCE_COLOR;
    this->m_title = "Color Live Stream";
    m_frameBuffer[0].reset(new ColorFrame(640, 480));
    m_frameBuffer[1].reset(new ColorFrame(640, 480));
    StreamInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
    m_openni = nullptr;
}

OpenNIColorInstance::~OpenNIColorInstance()
{
    closeInstance();
    m_openni = nullptr;
}

bool OpenNIColorInstance::is_open() const
{
    return m_openni != nullptr;
}

bool OpenNIColorInstance::openInstance()
{
    bool result = false;

    if (!is_open())
    {
        m_openni = OpenNIRuntime::getInstance();
        result = true;
    }

    return result;
}

void OpenNIColorInstance::closeInstance()
{
    if (is_open())
    {
        m_openni->releaseInstance();
        m_openni = nullptr;
    }
}

void OpenNIColorInstance::restartInstance()
{

}

void OpenNIColorInstance::nextFrame(ColorFrame &frame)
{
    openni::VideoFrameRef oniColorFrame = m_openni->readColorFrame();

    // Read this frame
    frame.setIndex(oniColorFrame.getFrameIndex());
    memcpy((void*) frame.getDataPtr(), oniColorFrame.getData(), 640 * 480 * sizeof(openni::RGB888Pixel));

    // Stats
    computeStats(frame.getIndex());
}

} // End namespace
