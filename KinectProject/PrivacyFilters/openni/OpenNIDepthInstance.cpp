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
        m_openni->addNewDepthListener(this);
        result = true;
    }

    return result;
}

void OpenNIDepthInstance::closeInstance()
{
    if (is_open())
    {
        m_openni->removeDepthListener(this);
        m_openni->releaseInstance();
        m_openni = nullptr;
    }
}

void OpenNIDepthInstance::restartInstance()
{

}

void OpenNIDepthInstance::nextFrame(DepthFrame &frame)
{
    // Wait until OpenNI provide a new frame
    waitForNewFrame();

    // Read this frame
    QMutexLocker locker(&m_lockFrame);
    const openni::DepthPixel* pDepth = (const openni::DepthPixel*) m_oniDepthFrame.getData();
    frame.setIndex(m_oniDepthFrame.getFrameIndex());

    for (int y=0; y < m_oniDepthFrame.getHeight(); ++y) {
        for (int x=0; x < m_oniDepthFrame.getWidth(); ++x) {
            frame.setItem(y, x, *pDepth / 1000.0f);
            pDepth++;
        }
        // Skip rest of row (in case it exists)
        //pDepth += strideDepth;
    }

    // Stats
    computeStats(frame.getIndex());
}

void OpenNIDepthInstance::onNewFrame(openni::VideoStream& stream)
{
    m_lockFrame.lock();

    if (stream.readFrame(&m_oniDepthFrame) != openni::STATUS_OK) {
        throw 1;
    }

    if (!m_oniDepthFrame.isValid()) {
        throw 2;
    }

    int strideDepth = m_oniDepthFrame.getStrideInBytes() / sizeof(openni::DepthPixel) - m_oniDepthFrame.getWidth();

    if (strideDepth > 0) {
        qWarning() << "WARNING: OpenNIRuntime - Not managed depth stride!!!";
        throw 3;
    }

    m_lockFrame.unlock();

    notifyNewFrame();
}

} // End namespace
