#include "StreamInstance.h"

namespace dai {

StreamInstance::StreamInstance(DataFrame::SupportedFrames supportedFrames)
{
    m_supportedFrames = supportedFrames;
    m_frameIndex = 0;
}

unsigned int StreamInstance::getFrameIndex() const
{
    return m_frameIndex - 1;
}

DataFrame::SupportedFrames StreamInstance::getSupportedFrames() const
{
    return m_supportedFrames;
}

void StreamInstance::open()
{
    if (!is_open()) {
        m_frameIndex = 0;
        if (!openInstance()) {
            throw CannotOpenInstanceException();
        }
    }
}

void StreamInstance::close()
{
    if (is_open()) {
        closeInstance();
    }
}

void StreamInstance::restart()
{
    if (is_open()) {
        restartInstance();
        m_frameIndex = 0;
    }
}

void StreamInstance::readNextFrame()
{
    if (!is_open()) {
        throw NotOpenedInstanceException();
    }

    if (hasNext()) {
        QWriteLocker locker(&m_locker);
        m_readFrames = nextFrame();
        m_frameIndex++;
    }
    else {
        closeInstance();
    }
}

bool StreamInstance::hasNext() const
{
    return true;
}

QList<shared_ptr<DataFrame>> StreamInstance::frames()
{
    QReadLocker locker(&m_locker);
    return m_readFrames;
}

} // End Namespace
