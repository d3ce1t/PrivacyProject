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
    qint64 availableTime;
    qint64 remainingTime = 0;
    qint64 averageTime = 0;
    qint64 offsetTime = 0;

    m_running = true;
    setupListeners();
    openAllInstances();

    while (m_running)
    {
        // Time: How much time I have?
        timer.start();
        availableTime = m_slotTime + offsetTime;

        // Do something
        bool hasProduced = generate();

        if (!hasProduced || subscribersCount() == 0)
            m_running = false;

        // Time management: Do I have free time?
        remainingTime = availableTime - timer.nsecsElapsed();

        if (remainingTime > 0)
            QThread::currentThread()->usleep(remainingTime / 1000);

        averageTime += timer.nsecsElapsed();;
        offsetTime = availableTime - timer.nsecsElapsed();
    }

    closeAllInstances();

    qint64 pCounter = productsCount();

    if (pCounter > 0)
        qDebug() << "Average Time:" << (averageTime / pCounter) / 1000 << "Frame Count:" << pCounter;
}

void PlaybackWorker::stop()
{
    m_running = false;
}

void PlaybackWorker::setupListeners()
{
    if (QThreadPool::globalInstance()->maxThreadCount() < subscribersCount())
        QThreadPool::globalInstance()->setMaxThreadCount(subscribersCount());
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
inline QHashDataFrames PlaybackWorker::produceFrames()
{
    QList<shared_ptr<StreamInstance>> instances = m_instances; // implicit sharing
    QHashDataFrames readFrames;

    foreach (shared_ptr<StreamInstance> instance, instances)
    {
        if (instance->is_open())
        {
            if (!instance->hasNext() && m_playloop_enabled)
                instance->restart();

            if (instance->hasNext())
            {
                instance->readNextFrames();
                QList<shared_ptr<DataFrame>> frames = instance->frames();

                foreach (shared_ptr<DataFrame> frame, frames) {
                    readFrames.insert(frame->getType(), frame);
                }
            }
            else {
                instance->close();
                qDebug() << "Closed";
            }
        }
    }

    return readFrames;
}

} // End Namespace
