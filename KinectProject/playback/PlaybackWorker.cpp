#include "PlaybackWorker.h"
#include "PlaybackControl.h"
#include <QThread>
#include <QElapsedTimer>
#include <iostream>

namespace dai {

PlaybackWorker::PlaybackWorker(PlaybackControl* parent)
    : m_sleepTime(100)
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
    // Initial Swap

    while (m_running)
    {
        // Compute time since last update
        qint64 timeNow = time.elapsed();
        qint64 diffTime = timeNow - lastTime;

        if (diffTime >= m_sleepTime)
        {
            // Compute Frame Per Seconds
            m_fps = 1.0 / (diffTime / 1000.0f);
            lastTime = timeNow;

            // Notify listeners; Here

            // Prefetch read
            QList<shared_ptr<StreamInstance>> readInstances = m_parent->readAllInstances();

            // WaitForNotifiers

            // Swap

            // Notify
            if (readInstances.count() > 0) {                
                emit availableInstances(readInstances);
                waitForNotifier();
            }
            else {
                m_running = false;
            }
        }
        else {
            QThread::currentThread()->msleep(m_sleepTime - diffTime);
        }
    }

    emit finished();
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
