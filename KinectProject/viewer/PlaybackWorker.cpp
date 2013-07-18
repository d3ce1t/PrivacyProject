#include "PlaybackWorker.h"
#include "viewer/PlaybackControl.h"
#include <iostream>

namespace dai {

PlaybackWorker::PlaybackWorker(PlaybackControl* parent)
    : SLEEP_TIME(100)
{
    m_parent = parent;
    m_running = false;
}

PlaybackWorker::~PlaybackWorker()
{
    QMutexLocker locker(&m_lockViewers);
    m_currentCallers.clear();
}

void PlaybackWorker::run()
{
    m_frames = 0;
    m_fps = 0;
    m_lastTime = 0;
    m_time.start();
    m_running = true;

    while (m_running)
    {
        // Compute time since last update
        qint64 timeNow = m_time.elapsed();
        qint64 diffTime = timeNow - m_lastTime;

        if (diffTime >= SLEEP_TIME)
        {
            // Compute Frame Per Seconds
            m_frames++;
            m_fps = 1.0 / (diffTime / 1000.0f);
            m_lastTime = timeNow;

            // Do job
            m_running = m_parent->doWork();

            // Wait
            /*m_viewers.ref();
            if (m_viewers.deref()) {*/
               /* m_mutex.lock();
                m_sync.wait(&m_mutex);
                m_mutex.unlock();*/
            /*    SLEEP_TIME = 100;
            } else {
                SLEEP_TIME = 10;
            }*/
        }
        else {
            this->msleep(SLEEP_TIME - diffTime);
        }
    }

    QMetaObject::invokeMethod(m_parent, "stopAsync", Qt::AutoConnection);
}

void PlaybackWorker::stop()
{
    m_running = false;
    std::cerr << "PlaybackWorker::stop()" << std::endl;
}

void PlaybackWorker::sync()
{
    m_mutex.lock();
    m_sync.wakeOne();
    m_mutex.unlock();
}

void PlaybackWorker::acquire(PlaybackControl::PlaybackListener *caller)
{
    //std::cerr << "PlaybackWorker::acquire" << std::endl;
    QMutexLocker locker(&m_lockViewers);
    m_viewers++;
    m_currentCallers.insert(caller, true);
}

void PlaybackWorker::release(PlaybackControl::PlaybackListener *caller)
{
    //std::cerr << "PlaybackWorker::release" << std::endl;
    QMutexLocker locker(&m_lockViewers);

    if (m_currentCallers.contains(caller)) {
        m_currentCallers.remove(caller);
        m_viewers--;
        if (m_viewers == 0)
            sync();
    }
}

} // End namespace
