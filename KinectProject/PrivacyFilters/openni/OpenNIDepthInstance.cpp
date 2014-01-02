#include "OpenNIDepthInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIDepthInstance::OpenNIDepthInstance()
{
    this->m_type = dai::INSTANCE_DEPTH;
    this->m_title = "Depth Live Stream";
    m_frameBuffer[0].reset(new DepthFrame(640, 480));
    m_frameBuffer[1].reset(new DepthFrame(640, 480));
    StreamInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
    m_openni = nullptr;
}

OpenNIDepthInstance::~OpenNIDepthInstance()
{
    closeInstance();
    m_openni = nullptr;
}

bool OpenNIDepthInstance::is_open() const
{
    return m_openni != nullptr;
}

bool OpenNIDepthInstance::openInstance()
{
    bool result = false;

    if (!is_open())
    {
        m_openni = OpenNIRuntime::getInstance();
        result = true;
    }

    return result;
}

void OpenNIDepthInstance::closeInstance()
{
    if (is_open())
    {
        m_openni->releaseInstance();
        m_openni = nullptr;
    }
}

void OpenNIDepthInstance::restartInstance()
{

}

void OpenNIDepthInstance::nextFrame(DepthFrame &frame)
{
    openni::VideoFrameRef oniDepthFrame = m_openni->readDepthFrame();

    // Read this frame
    const openni::DepthPixel* pDepth = (const openni::DepthPixel*) oniDepthFrame.getData();
    frame.setIndex(oniDepthFrame.getFrameIndex());

    for (int y=0; y < oniDepthFrame.getHeight(); ++y) {
        for (int x=0; x < oniDepthFrame.getWidth(); ++x) {
            frame.setItem(y, x, *pDepth / 1000.0f);
            pDepth++;
        }
        // Skip rest of row (in case it exists)
        //pDepth += strideDepth;
    }

    // Stats
    computeStats(frame.getIndex());
}

} // End namespace
