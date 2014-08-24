#include "FrameListener.h"
#include "FrameNotifier.h"
#include <QDebug>

namespace dai {

FrameListener::FrameListener()
    : m_worker(nullptr)
    , m_lastFrameId(0)
{
}

FrameListener::~FrameListener()
{
    qDebug() << "FrameListener::~FrameListener";
    stopListener();
    m_worker = nullptr;
}

bool FrameListener::hasExpired()
{
    bool result = true;

    if (m_worker)
        result = !m_worker->isValidFrame(m_lastFrameId);

    return result;
}

FrameGenerator *FrameListener::producerHandler()
{
    return m_worker;
}

void FrameListener::stopListener()
{
    if (m_worker)
        m_worker->removeListener(this); // Blocks until last job ends
}

void FrameListener::newFrames(const QHashDataFrames dataFrames, const qint64 frameId)
{
    // Check the received frames are valid because we could have been called out of time
    // frameId is the frame counter of the frame generator.
    if (!m_worker->isValidFrame(frameId)) {
        qDebug() << "FrameListener - Frame" << frameId << "Skipped";
        return;
    }

    m_lastFrameId = frameId;
    newFrames(dataFrames);
}

} // End Namespace
