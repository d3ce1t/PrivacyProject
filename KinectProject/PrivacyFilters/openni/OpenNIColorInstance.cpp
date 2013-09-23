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
        m_openni->addNewColorListener(this);
        result = true;
    }

    return result;
}

void OpenNIColorInstance::closeInstance()
{
    if (is_open())
    {
        m_openni->removeColorListener(this);
        m_openni->releaseInstance();
        m_openni = nullptr;
    }
}

void OpenNIColorInstance::restartInstance()
{

}

void OpenNIColorInstance::nextFrame(ColorFrame &frame)
{
    // Wait until OpenNI provide a new frame
    waitForNewFrame();

    // Read this frame
    QMutexLocker locker(&m_lockFrame);
    frame.setIndex(m_oniColorFrame.getFrameIndex());
    memcpy((void*) frame.getDataPtr(), m_oniColorFrame.getData(), 640 * 480 * sizeof(openni::RGB888Pixel));

    // Stats
    computeStats(frame.getIndex());
}

// Called from OpenNI frame listener thread
void OpenNIColorInstance::onNewFrame(openni::VideoStream& stream)
{
    m_lockFrame.lock();

    if (stream.readFrame(&m_oniColorFrame) != openni::STATUS_OK)
        throw 1;

    if (!m_oniColorFrame.isValid())
        throw 2;

    int stride = m_oniColorFrame.getStrideInBytes() / sizeof(openni::RGB888Pixel) - m_oniColorFrame.getWidth();

    if (stride > 0) {
        qWarning() << "WARNING: OpenNIRuntime - Not managed color stride!!!";
        throw 3;
    }

    m_lockFrame.unlock();

    notifyNewFrame();
}

} // End namespace
