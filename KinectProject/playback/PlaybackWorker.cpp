#include "PlaybackWorker.h"
#include "PlaybackControl.h"
#include <QElapsedTimer>
#include <iostream>

namespace dai {

PlaybackWorker::PlaybackWorker(PlaybackControl* parent)
    : m_sleepTime(100)
{
    m_parent = parent;
    m_running = false;
}

void PlaybackWorker::run()
{
    QElapsedTimer time;
    qint64 lastTime = 0;

    m_fps = 0;
    m_running = true;
    time.start();

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

            // Do job
            m_running = m_parent->readAllInstances();
        }
        else {
            this->msleep(m_sleepTime - diffTime);
        }
    }

    QMetaObject::invokeMethod(m_parent, "stopAsync", Qt::AutoConnection);
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
