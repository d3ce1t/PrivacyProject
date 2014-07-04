#include "NodeProducer.h"
#include "NodeListener.h"
#include <QtConcurrent/QtConcurrent>

namespace dai {

NodeProducer::NodeProducer()
    : m_productsCounter(0)
    , m_lastTime(0)
{
    m_timer.start();
}

NodeProducer::~NodeProducer()
{
    m_listeners.clear();
}

int NodeProducer::subscribersCount() const
{
    return m_listeners.size();
}

void NodeProducer::addListener(NodeListener* listener)
{
    QWriteLocker locker(&m_listenersLock);

    if (!m_listeners.contains(listener)) {
        listener->m_worker = this;
        m_listeners << listener;
    }
}

void NodeProducer::removeListener(NodeListener* listener)
{
    QWriteLocker locker(&m_listenersLock);

    if (m_listeners.contains(listener)) {
        m_listeners.removeOne(listener);
    }
}

// Devuelve True si ha producido y false en caso contrario
bool NodeProducer::produce()
{
    bool hasProduced = false;

    qint64 timeBetweenInvocations = m_timer.nsecsElapsed() - m_lastTime;
    m_lastTime = m_timer.nsecsElapsed();
    m_productionRate = 1000000000 / timeBetweenInvocations;

    // Frames counter
    m_counterLock.lockForWrite();
    m_productsCounter++;
    m_counterLock.unlock();

    QHashDataFrames readFrames = produceFrames();

    // Notify listeners
    if (readFrames.size() > 0) {
        notifyListeners(readFrames);
        hasProduced = true;
    }

    qint64 spentTime = m_timer.nsecsElapsed() - m_lastTime;
    m_instantProductionRate = 1000000000 / spentTime;
    return hasProduced;
}

float NodeProducer::getProductionCapacity() const
{
    return m_instantProductionRate;
}

float NodeProducer::getProductionRate() const
{
    return m_productionRate;
}

qint64 NodeProducer::productsCount()
{
    QReadLocker locker(&m_counterLock);
    return m_productsCounter;
}

bool NodeProducer::isValidProduct(qint64 frameIndex)
{
    bool result = false;

    m_counterLock.lockForRead();
    if (frameIndex == m_productsCounter) {
        result = true;
    }
    m_counterLock.unlock();

    return result;
}

// Devuelve true si hay listeners a los que notificar, false en caso contrario.
void NodeProducer::notifyListeners(const QHashDataFrames dataFrames)
{
    // Notify listeners (Time is measured since this method is called and until the notification is received)
    // signals and slots, debug,   25 fps, Max 29.46 (ms), Min 0.05 (ms), Avg 14.44 (ms)
    // New Approach: Max (ms) 6.08369 Min (ms) 1.02753 Avg (ms) 1.50493
    QReadLocker locker(&m_listenersLock);
    QReadLocker counterLocker(&m_counterLock);

    foreach (NodeListener* listener, m_listeners) {
        QFuture<void> future = listener->m_future;
        if (future.isFinished()) {
            listener->m_future = QtConcurrent::run(listener, &NodeListener::newFrames, dataFrames, m_productsCounter);
        }
    }
}

} // End Namespace
