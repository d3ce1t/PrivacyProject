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

// Al destruirme, espero a que el último trabajo finalice
FrameNotifier::~FrameNotifier()
{
    stop();
    m_listener = nullptr;
    qDebug() << "FrameNotifier::~FrameNotifier()";
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

    if (QThread::currentThread() != this)
        this->wait();
}

void FrameNotifier::run()
{
    while (m_running)
    {
        if (waitingForNewOrder()) {
            m_listener->newFrames(m_data, m_frameId);
        }
        done();
    }

    m_listener->afterStop();
    qDebug() << "FrameNotifier::run() is over";
}

// While the notifier is working, new notifications are ignored
void FrameNotifier::notifyListener(const QHashDataFrames& data, const qint64 frameId)
{
    m_syncLock.lock();
    if (!m_workInProgress) {
        m_data = data; // Implicit copy
        m_frameId = frameId;
        m_workInProgress = true;
        m_sync.wakeOne();
    } else {
        qDebug() << "FrameNotifier::notifyListener() ignored (work in progress pending)";
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
