#include "OpenNIBaseInstance.h"
#include <QDebug>

namespace dai {

OpenNIBaseInstance::OpenNIBaseInstance()
{
    m_lastFrameId = 0;
    m_newFrameGenerated = false;
}

void OpenNIBaseInstance::computeStats(unsigned int frameIndex)
{
    static int skippedFrames = 0;

    if (frameIndex > m_lastFrameId + 1) {
        skippedFrames += frameIndex - (m_lastFrameId + 1);
        //qDebug() << "Skip frames" << skippedFrames;
    }

    m_lastFrameId = frameIndex;
}

void OpenNIBaseInstance::notifyNewFrame()
{
    m_lockSync.lock();
    if (!m_newFrameGenerated) {
        m_newFrameGenerated = true;
        m_sync.wakeOne();
    }
    m_lockSync.unlock();
}

void OpenNIBaseInstance::waitForNewFrame()
{
    m_lockSync.lock();
    while (!m_newFrameGenerated) {
        m_sync.wait(&m_lockSync);
    }
    m_newFrameGenerated = false;
    m_lockSync.unlock();
}

} // End namespace
