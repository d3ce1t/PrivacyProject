#include "PlaybackWorker.h"
#include "PlaybackControl.h"
#include <QElapsedTimer>
#include "exceptions/CannotOpenInstanceException.h"
#include <iostream>
#include <QDebug>

namespace dai {

PlaybackWorker::PlaybackWorker(const PlaybackControl* playback, QList<shared_ptr<BaseInstance> > &instances)
    : m_playback(playback)
    , m_instances(instances)
    , m_playloop_enabled(false)
    , m_slotTime(40 * 1000000) // 40 ms in ns
    , m_running(false)
    , m_fps(0)
{
}

void PlaybackWorker::enablePlayLoop(bool value)
{
    m_playloop_enabled = value;
}

void PlaybackWorker::setFPS(float fps)
{
    m_slotTime = 1000000000 / fps;
}

float PlaybackWorker::getFPS() const
{
   return m_fps;
}

bool PlaybackWorker::isValidFrame(qint64 frameIndex)
{
    bool result = false;

    m_lock.lockForRead();
    if (frameIndex == m_framesCounter) {
        result = true;
    }
    m_lock.unlock();

    return result;
}

/**
 * @brief PlaybackControl::run
 *
 * We assume that the reading from instances takes less time that the slot time available.
 * If this assumption does not fulfil, time restrictions are not satisfied.
 */
void PlaybackWorker::run()
{
    QList<shared_ptr<BaseInstance>> readInstances;
    QElapsedTimer timer;
    qint64 remainingTime = 0;
    qint64 totalTime = 0;
    qint64 averageTime = 0;
    m_framesCounter = 0;
    qint64 offsetTime = 0;

    m_running = true;
    openAllInstances();
    timer.start();

    while (m_running)
    {
        // Retrieve instances from which frames has been read
        readInstances = readAllInstances();

        if (readInstances.count() > 0)
        {
            // Frames counter
            m_lock.lockForWrite();
            m_framesCounter++;
            m_lock.unlock();

            // Swap buffers, write buffer becomes the read buffer and vice versa
            swapAllInstances(readInstances);

            // Prepare data
            QMultiHash<DataFrame::FrameType, shared_ptr<DataFrame>> readFrames;

            foreach (shared_ptr<BaseInstance> instance, readInstances) {
                QList<shared_ptr<DataFrame>> frames = instance->frames();
                foreach (shared_ptr<DataFrame> frame, frames) {
                    readFrames.insert(frame->getType(), frame);
                }
            }

            // Emit Signal
            emit onNewFrames(readFrames, m_framesCounter, m_playback);

            // Time management: Do I have time?
            remainingTime = m_slotTime - timer.nsecsElapsed() + offsetTime;

            if (remainingTime > 0)
                QThread::currentThread()->usleep(remainingTime / 1000);

            totalTime = timer.nsecsElapsed();
            timer.restart();
            averageTime += totalTime;
            m_fps = 1000000000 / totalTime;
            offsetTime = m_slotTime - totalTime;

            /*if (m_framesCounter >= 100)
                m_running = false;*/
        }
        else {
            m_running = false;
        }
    }

    closeAllInstances();
    qDebug() << "Average Time:" << (averageTime / m_framesCounter) / 1000 << "Frame Count:" << m_framesCounter;
    emit onStop();
}

void PlaybackWorker::stop()
{
    m_running = false;
}

inline void PlaybackWorker::openAllInstances()
{
    // Open all instances
    QListIterator<shared_ptr<BaseInstance>> it(m_instances);

    while (it.hasNext())
    {
        shared_ptr<BaseInstance> instance = it.next();

        if (!instance->is_open()) {
            try {
                instance->open();
                std::cerr << "Open" << std::endl;
            }
            catch (CannotOpenInstanceException ex) {
                throw ex;
            }
        }
    }
}

inline void PlaybackWorker::closeAllInstances()
{
    // Close all opened instances
    QListIterator<shared_ptr<BaseInstance> > it(m_instances);

    while (it.hasNext()) {
        auto instance = it.next();
        if (instance->is_open()) {
            instance->close();
            std::cerr << "Close" << std::endl;
        }
    }
}

inline QList<shared_ptr<BaseInstance>> PlaybackWorker::readAllInstances()
{
    QList<shared_ptr<BaseInstance>> instances = m_instances; // implicit sharing
    QList<shared_ptr<BaseInstance>> changedInstances;

    foreach (shared_ptr<BaseInstance> instance, instances)
    {
        if (instance->is_open())
        {
            if (!instance->hasNext() && m_playloop_enabled)
                instance->restart();

            if (instance->hasNext()) {
                instance->readNextFrame();
                changedInstances << instance;
            } else {
                instance->close();
                std::cerr << "Closed" << std::endl;
            }
        }
    }

    return changedInstances;
}

inline void PlaybackWorker::swapAllInstances(const QList<shared_ptr<BaseInstance> > &instances)
{
    foreach (shared_ptr<BaseInstance> instance, instances) {
        instance->swapBuffer();
    }
}

} // End Namespace
