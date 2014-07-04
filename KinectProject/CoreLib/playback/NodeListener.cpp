#include "NodeListener.h"
#include <QDebug>

namespace dai {

NodeListener::~NodeListener()
{
    qDebug() << "NodeListener::~NodeListener";
    stopListener();
}

bool NodeListener::hasExpired()
{
    return !m_worker->isValidProduct(m_lastFrameId);
}

NodeProducer *NodeListener::producerHandler()
{
    return m_worker;
}

void NodeListener::stopListener()
{
    m_worker->removeListener(this);
    m_future.waitForFinished();
}

void NodeListener::newFrames(const QHashDataFrames dataFrames, const qint64 frameId)
{
    // Check the received frames are valid because we could have been called out of time
    if (!m_worker->isValidProduct(frameId)) {
        qDebug() << "Frame Id:" << frameId << "Skipped";
        return;
    }

    m_lastFrameId = frameId;
    newFrames(dataFrames);
}

} // End Namespace
