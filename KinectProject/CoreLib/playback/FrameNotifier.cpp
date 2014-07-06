#include "FrameNotifier.h"
#include "FrameListener.h"
#include <QDebug>

namespace dai {

FrameNotifier::FrameNotifier(FrameListener *listener)
    : m_listener(listener)
    , m_frameId(0)
    , m_running(true)
    , m_workInProgress(false)
{
}

// Al destroirme, espero a que el último trabajo finalice
FrameNotifier::~FrameNotifier()
{
    stop();
    m_loopLock.lock();
    m_loopLock.unlock();
    m_listener = nullptr;
}

void FrameNotifier::stop()
{
    m_syncLock.lock();
    m_running = false;
    if (!m_workInProgress) {
        m_workInProgress = true;
        m_sync.wakeOne();
    }
    m_syncLock.unlock();
}

void FrameNotifier::run()
{
    m_loopLock.lock();

    while (m_running)
    {
        if (waitingForNewOrder()) {
            m_listener->newFrames(m_data, m_frameId);
        }
        done();
    }

    m_loopLock.unlock();
    qDebug() << "Notifier finished";
}

// While the notifier is working, new notifications are ignored
void FrameNotifier::notifyListener(const QHashDataFrames data, const qint64 frameId)
{
    m_syncLock.lock();
    if (!m_workInProgress) {
        m_data = data; // Implicit copy
        m_frameId = frameId;
        m_workInProgress = true;
        m_sync.wakeOne();
    }
    m_syncLock.unlock();
}

// Devuelve true si tiene que trabajar, o false en caso de que se haya cancelado
bool FrameNotifier::waitingForNewOrder()
{
    bool result = false;
    m_syncLock.lock();
    while (!m_workInProgress) {
        m_sync.wait(&m_syncLock);
    }
    result = m_running;
    m_syncLock.unlock();
    return result;
}

void FrameNotifier::done()
{
    m_syncLock.lock();
    m_workInProgress = false;
    m_syncLock.unlock();
}

} // End Namespace
