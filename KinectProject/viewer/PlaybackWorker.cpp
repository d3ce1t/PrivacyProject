#include "PlaybackWorker.h"
#include "viewer/PlaybackControl.h"
#include <QDebug>

namespace dai {

PlaybackWorker::PlaybackWorker(PlaybackControl* parent)
    : SLEEP_TIME(70)
{
    m_parent = parent;
    m_running = false;
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
            m_parent->doWork();

            // Wait
            m_mutex.lock();
            m_sync.wait(&m_mutex);
            m_mutex.unlock();
        }
        else {
            this->msleep(SLEEP_TIME - diffTime);
        }
    }
}

void PlaybackWorker::stop()
{
    m_running = false;
    sync();
}

void PlaybackWorker::sync()
{
    m_mutex.lock();
    m_sync.wakeOne();
    m_mutex.unlock();
}

} // End namespace
