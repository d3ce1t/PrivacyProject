#include "FrameListener.h"
#include "FrameNotifier.h"
#include <QDebug>

namespace dai {

FrameListener::~FrameListener()
{
    qDebug() << "NodeListener::~NodeListener";
    stopListener();
}

bool FrameListener::hasExpired()
{
    return !m_worker->isValidFrame(m_lastFrameId);
}

FrameGenerator *FrameListener::producerHandler()
{
    return m_worker;
}

void FrameListener::stopListener()
{
    m_worker->removeListener(this); // Blocks until last job ends
}

void FrameListener::newFrames(const QHashDataFrames dataFrames, const qint64 frameId)
{
    // Check the received frames are valid because we could have been called out of time
    if (!m_worker->isValidFrame(frameId)) {
        qDebug() << "Frame Id:" << frameId << "Skipped";
        return;
    }

    m_lastFrameId = frameId;
    newFrames(dataFrames);
}

} // End Namespace
