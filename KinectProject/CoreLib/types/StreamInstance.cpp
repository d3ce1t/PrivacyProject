#include "StreamInstance.h"

namespace dai {

StreamInstance::StreamInstance(DataFrame::SupportedFrames supportedFrames)
{
    m_supportedFrames = supportedFrames;
    m_frame_counter = 0;
    m_info.width = 0;
    m_info.width = 0;
}

StreamInstance::StreamInstance(DataFrame::SupportedFrames supportedFrames, int width, int height)
{
    m_supportedFrames = supportedFrames;
    m_frame_counter = 0;
    m_info.width = width;
    m_info.height = height;
}

unsigned int StreamInstance::getFrameCounter() const
{
    return m_frame_counter - 1;
}

DataFrame::SupportedFrames StreamInstance::getSupportedFrames() const
{
    return m_supportedFrames;
}

const StreamInfo& StreamInstance::getStreamInfo() const
{
    return m_info;
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
    return is_open();
}

QList<DataFrame::FrameType> StreamInstance::getTypes(DataFrame::SupportedFrames type)
{
    DataFrame::FrameType all_types[] = {
        DataFrame::Color, DataFrame::Depth, DataFrame::Skeleton,
        DataFrame::Mask, DataFrame::Metadata
    };

    QList<DataFrame::FrameType> result;

    for (int i=0; i<5; ++i) {
        if (type.testFlag(all_types[i])) {
            result << all_types[i];
        }
    }

    return result;
}

} // End Namespace
