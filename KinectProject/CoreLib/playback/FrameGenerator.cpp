#include "FrameGenerator.h"
#include "FrameListener.h"
#include "FrameNotifier.h"


namespace dai {

FrameGenerator::FrameGenerator()
    : m_frameCounter(0)
{
    m_timer.start();
}

FrameGenerator::~FrameGenerator()
{
    m_listeners.clear();
}

int FrameGenerator::subscribersCount() const
{
    return m_listeners.size();
}

void FrameGenerator::addListener(FrameListener* listener)
{
    QWriteLocker locker(&m_listenersLock);

    if (!m_listeners.contains(listener)) {
        listener->m_worker = this;
        FrameNotifier* notifier = new FrameNotifier(listener);
        QObject::connect(notifier, &QThread::finished, notifier, &QThread::deleteLater);
        notifier->start();
        m_listeners.insert(listener, notifier);
    }
}

void FrameGenerator::removeListener(FrameListener* listener)
{
    QWriteLocker locker(&m_listenersLock);

    if (m_listeners.contains(listener)) {
        FrameNotifier* notifier = m_listeners.value(listener);
        notifier->stop();
        m_listeners.remove(listener);
    }
}

void FrameGenerator::notifyListeners(const QHashDataFrames& dataFrames, qint64 frameId)
{
    // Notify listeners (Time is measured since this method is called and until the notification is received)
    // signals and slots, debug,   25 fps, Max 29.46 (ms), Min 0.05 (ms), Avg 14.44 (ms)
    // New Approach: Max (ms) 6.08369 Min (ms) 1.02753 Avg (ms) 1.50493
    QReadLocker locker(&m_listenersLock);

    foreach (FrameListener* listener, m_listeners.keys()) {
        FrameNotifier* notifier = m_listeners.value(listener);
        notifier->notifyListener(dataFrames, frameId);
    }
}

// Devuelve True si ha producido y false en caso contrario
bool FrameGenerator::generate()
{
    bool hasProduced = false;

    // Stats 1
    qint64 timeBetweenInvocations = m_timer.nsecsElapsed();
    m_timer.start();
    m_productionRate = 1000000000.0f / timeBetweenInvocations;

    // Frames counter
    m_counterLock.lockForWrite();
    m_frameCounter++;
    m_counterLock.unlock();

    QHashDataFrames readFrames = produceFrames();

    // Notify listeners
    if (readFrames.size() > 0) {
        notifyListeners(readFrames, m_frameCounter);
        hasProduced = true;
    }

    // Stats 2
    qint64 spentTime = m_timer.nsecsElapsed();
    m_instantProductionRate = 1000000000.0f / spentTime;
    return hasProduced;
}

void FrameGenerator::restartStats()
{
    m_counterLock.lockForWrite();
    m_frameCounter = 0;
    m_counterLock.unlock();
}

qint64 FrameGenerator::productsCount()
{
    QReadLocker locker(&m_counterLock);
    return m_frameCounter;
}

} // End Namespace
