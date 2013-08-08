#include "PlaybackWorker.h"
#include "PlaybackControl.h"
#include <QThread>
#include <QElapsedTimer>
#include <iostream>

namespace dai {

PlaybackWorker::PlaybackWorker(PlaybackControl* parent)
    : m_sleepTime(33000) // 33333 microseconds = 33.33 ms
{
    m_parent = parent;
    m_running = false;
    m_notifierFinish = false;
}

void PlaybackWorker::initialise()
{
    m_thread = new QThread;
    m_notifier = new PlaybackNotifier(m_parent, this);
    m_notifier->moveToThread(m_thread);

    QObject::connect(this, SIGNAL(finished()), m_thread, SLOT(quit()));
    QObject::connect(this, SIGNAL(finished()), m_notifier, SLOT(deleteLater()));
    QObject::connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
    QObject::connect(this,
                     SIGNAL(availableInstances(QList<shared_ptr<BaseInstance> >)),
                     m_notifier,
                     SLOT(notifyListeners(QList<shared_ptr<BaseInstance> >)));
    m_thread->start();
}

void PlaybackWorker::run()
{
    QElapsedTimer time;
    qint64 lastTime = 0;

    initialise();
    m_fps = 0;
    m_running = true;
    time.start();

    // Initial Read; Here
    QList<shared_ptr<BaseInstance>> readInstances = m_parent->readAllInstances();

    // Initial Swap
    swap(readInstances);

    while (m_running)
    {
        // Compute time since last update and fps
        qint64 timeNow = time.nsecsElapsed() / 1000; // nano seconds to microseconds
        qint64 diffTime = timeNow - lastTime;
        m_fps = 1.0 / (diffTime / 1000000.0f); // in seconds

        if (diffTime < m_sleepTime) {
            QThread::currentThread()->usleep(m_sleepTime - diffTime); // microseconds
        }
        else if (readInstances.count() > 0)
        {
            // Notify listeners
            emit availableInstances(readInstances);

            // Prefetch read
            readInstances = m_parent->readAllInstances();

            // WaitForNotifiers
            waitForNotifier();

            // Swap
            swap(readInstances);

            // Set last time
            lastTime = timeNow;
        }
        else {
            m_running = false;
        }
    }

    emit finished();
}

void PlaybackWorker::swap(const QList<shared_ptr<BaseInstance> > &instances)
{
    foreach (shared_ptr<BaseInstance> instance, instances) {
        instance->swapBuffer();
    }
}

void PlaybackWorker::sync()
{
    m_lockSync.lock();
    if (!m_notifierFinish) {
        m_notifierFinish = true;
        m_sync.wakeOne();
    }
    m_lockSync.unlock();
}

void PlaybackWorker::waitForNotifier()
{
    m_lockSync.lock();
    while (!m_notifierFinish) {
        m_sync.wait(&m_lockSync);
    }
    m_notifierFinish = false;
    m_lockSync.unlock();
}


void PlaybackWorker::stop()
{
    m_running = false;
    std::cerr << "PlaybackWorker::stop()" << std::endl;
}

float PlaybackWorker::getFPS() const
{
    return m_fps;
}

} // End namespace
