#include "PlaybackWorker.h"
#include "PlaybackControl.h"
#include <QThread>
#include <QElapsedTimer>
#include <iostream>

namespace dai {

PlaybackWorker::PlaybackWorker(PlaybackControl* parent)
    : m_sleepTime(40000) // 40000 microseconds = 40 ms
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
                     SIGNAL(availableInstances(QList<shared_ptr<StreamInstance>>)),
                     m_notifier,
                     SLOT(notifyListeners(QList<shared_ptr<StreamInstance>>)));
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
    QList<shared_ptr<StreamInstance>> readInstances = m_parent->readAllInstances();

    // Initial Swap
    swap(readInstances);

    while (m_running)
    {
        // Compute time since last update
        qint64 timeNow = time.nsecsElapsed() / 1000; // nano seconds to microseconds
        qint64 diffTime = timeNow - lastTime;

        if (diffTime >= m_sleepTime)
        {
            // Notify listeners; Here
            if (readInstances.count() > 0) {
                emit availableInstances(readInstances);
            }
            else {
                m_running = false;
            }

            // Prefetch read
            readInstances = m_parent->readAllInstances();

            // WaitForNotifiers
            waitForNotifier();

            // Swap
            swap(readInstances);

            // Compute Frame Per Seconds
            m_fps = 1.0 / (diffTime / 1000000.0f);
            lastTime = timeNow;
        }
        else {
            QThread::currentThread()->usleep(m_sleepTime - diffTime); // microseconds
        }
    }

    emit finished();
}

void PlaybackWorker::swap(QList<shared_ptr<StreamInstance>> instances)
{
    foreach (shared_ptr<StreamInstance> instance, instances) {
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
