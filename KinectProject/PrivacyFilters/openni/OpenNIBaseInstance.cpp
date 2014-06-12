#include "OpenNIBaseInstance.h"
#include <QDebug>

namespace dai {

OpenNIBaseInstance::OpenNIBaseInstance()
{
    m_lastFrameId = 0;
    m_skippedFrames = 0;
}

 unsigned int OpenNIBaseInstance::getSkippedFramesNumber() const
 {
     return m_skippedFrames;
 }

void OpenNIBaseInstance::computeStats(unsigned int frameIndex)
{
    if (frameIndex > m_lastFrameId + 1) {
        m_skippedFrames += frameIndex - (m_lastFrameId + 1);
    }

    m_lastFrameId = frameIndex;
}

} // End namespace
