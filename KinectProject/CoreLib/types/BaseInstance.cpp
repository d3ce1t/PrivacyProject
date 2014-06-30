#include "BaseInstance.h"

namespace dai {

BaseInstance::BaseInstance(DataFrame::SupportedFrames supportedFrames)
{
    m_supportedFrames = supportedFrames;
}

const QString& BaseInstance::getTitle() const
{
    return m_title;
}

DataFrame::SupportedFrames BaseInstance::getSupportedFrames() const
{
    return m_supportedFrames;
}

} // End Namespace
