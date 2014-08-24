#include "PlaybackWorker.h"
#include "FrameListener.h"
#include <QtConcurrent/QtConcurrent>
#include "exceptions/CannotOpenInstanceException.h"
#include <QDebug>

namespace dai {

PlaybackWorker::PlaybackWorker()
    : m_playloop_enabled(false)
    , m_slotTime(40 * 1000000) // 40 ms in ns, 25 fps
    , m_running(false)
    , m_supportedFrames(DataFrame::Unknown)
{
    superTimer.start();
}

PlaybackWorker::~PlaybackWorker()
{
    m_instances.clear();
    m_supportedFrames = DataFrame::Unknown;
}

bool PlaybackWorker::addInstance(shared_ptr<StreamInstance> instance)
{
    bool added = false;

    if (!m_instances.contains(instance))
    {
        DataFrame::SupportedFrames result = m_supportedFrames & instance->getSupportedFrames();

        if ( int(result) == 0) {
            m_instances << instance;
            m_supportedFrames |= instance->getSupportedFrames();
            added = true;
        } else {
            qDebug() << "WARNING" << "The provided instance cannot be added because it generates the"
                     << "same type of frame than another instance";
        }
    }
    else {
        qDebug() << "The provided instance already exists";
    }

    return added;
}

void PlaybackWorker::removeInstance(shared_ptr<StreamInstance> instance)
{
    if (m_instances.contains(instance)) {
        m_instances.removeOne(instance);
        m_supportedFrames ^= instance->getSupportedFrames();
    }
}

void PlaybackWorker::clearInstances()
{
    m_instances.clear();
    m_supportedFrames = DataFrame::Unknown;
}

void PlaybackWorker::enablePlayLoop(bool value)
{
    m_playloop_enabled = value;
}

void PlaybackWorker::setFPS(float fps)
{
    m_slotTime = 1000000000 / fps;
}

/**
 * @brief PlaybackControl::run
 *
 * We assume that the reading from instances takes less time that the slot time available.
 * If this assumption does not fulfil, time restrictions are not satisfied.
 */
void PlaybackWorker::run()
{
    QElapsedTimer timer;
    qint64 remainingTime = 0;
    qint64 totalTime = 0;
    qint64 global_deviation = 0;
    qint64 skip_counter = 0;

    restartStats();
    allocateMemory();
    openAllInstances();
    m_running = true;

    timer.start();

    while (m_running)
    {
        if (global_deviation >= -m_slotTime)
        {
            // Do something
            bool hasProduced = generate();

            if (!hasProduced || subscribersCount() == 0)
                m_running = false;

            // Time management: Do I have free time?
            remainingTime = m_slotTime - timer.nsecsElapsed();

            if (remainingTime > 1500000) {
                QThread::currentThread()->usleep((remainingTime - 1000000) / 1000); // FIX: usleep resolution is between 1 ms
            }
        }
        else {
            qDebug() << "PlaybackWorker - Skip Frame" << productsCount();
            skip_counter++;
        }

        totalTime += timer.nsecsElapsed();
        global_deviation += m_slotTime - timer.nsecsElapsed();

        if ( (productsCount() + skip_counter) % 100 == 0) {
            qDebug() << "PlaybackWorker is running" << productsCount() << "fps" << getFrameRate()
                     << "capacity" << getGeneratorCapacity();

            /*qDebug() << "Available Time (ms)" << m_slotTime / 1000000.0f
                     << "Remaining (ms)" << remainingTime / 1000000.0f
                     << "Deviation (ms)" << global_deviation / 1000000.0f;*/
        }

        timer.start();
    }

    closeAllInstances();

    qint64 pCounter = productsCount() + skip_counter;

    if (pCounter > 0)
        qDebug() << "Average Time:" << (totalTime / pCounter) / 1000 << "Frame Count:" << pCounter << "Skip" << skip_counter;
}

void PlaybackWorker::stop()
{
    m_running = false;
}

void PlaybackWorker::allocateMemory()
{
    foreach (shared_ptr<StreamInstance> instance, m_instances) {
        QList<DataFrame::FrameType> types = StreamInstance::getTypes(instance->getSupportedFrames());
        const StreamInfo& info = instance->getStreamInfo();
        foreach (DataFrame::FrameType type, types) {
            m_frames.insert(type, DataFrame::create(type, info.width, info.height));
        }
    }
}

inline void PlaybackWorker::openAllInstances()
{
    // Open all instances
    QListIterator<shared_ptr<StreamInstance>> it(m_instances);

    while (it.hasNext())
    {
        shared_ptr<StreamInstance> instance = it.next();

        if (!instance->is_open()) {
            try {
                instance->open();
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
    QListIterator<shared_ptr<StreamInstance> > it(m_instances);

    while (it.hasNext()) {
        auto instance = it.next();
        if (instance->is_open()) {
            instance->close();
        }
    }
}

// Debug:   20 ms
// Release: 10 ms
QHashDataFrames PlaybackWorker::produceFrames()
{
    QList<shared_ptr<StreamInstance>> instances = m_instances; // implicit sharing

    foreach (shared_ptr<StreamInstance> instance, instances)
    {
        bool hasNext = instance->hasNext();

        if (!hasNext && m_playloop_enabled) {
            instance->restart();
            hasNext = instance->hasNext();
        }

        if (hasNext) {
            instance->readNextFrame(m_frames);
        }
        else {
            instance->close();
            qDebug() << "Closed";
        }
    }

    return m_frames;
}

} // End Namespace
