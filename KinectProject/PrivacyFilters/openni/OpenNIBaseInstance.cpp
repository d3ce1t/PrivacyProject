#include "OpenNIBaseInstance.h"
#include <QDebug>

namespace dai {

OpenNIBaseInstance::OpenNIBaseInstance()
{
    m_lastFrameId = 0;
}

void OpenNIBaseInstance::computeStats(unsigned int frameIndex)
{
    static int skippedFrames = 0;

    if (frameIndex > m_lastFrameId + 1) {
        skippedFrames += frameIndex - (m_lastFrameId + 1);
    }

    m_lastFrameId = frameIndex;
}

} // End namespace
