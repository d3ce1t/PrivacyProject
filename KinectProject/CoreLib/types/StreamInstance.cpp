#include "StreamInstance.h"

namespace dai {

StreamInstance::StreamInstance(DataFrame::SupportedFrames supportedFrames)
{
    m_supportedFrames = supportedFrames;
    m_frame_counter = 0;
}

unsigned int StreamInstance::getFrameCounter() const
{
    return m_frame_counter - 1;
}

DataFrame::SupportedFrames StreamInstance::getSupportedFrames() const
{
    return m_supportedFrames;
}

void StreamInstance::open()
{
    if (!is_open()) {
        m_frame_counter = 0;
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
        m_frame_counter = 0;
    }
}

bool StreamInstance::hasNext() const
{
    return true;
}

} // End Namespace
