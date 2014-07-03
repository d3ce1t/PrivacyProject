#include "PlaybackWorker.h"
#include "PlaybackListener.h"
#include <QtConcurrent/QtConcurrent>
#include "exceptions/CannotOpenInstanceException.h"
#include <QDebug>

namespace dai {

PlaybackWorker::PlaybackWorker()
    : m_playloop_enabled(false)
    , m_slotTime(40 * 1000000) // 40 ms in ns, 25 fps
    , m_running(false)
    , m_fps(0)
    , m_supportedFrames(DataFrame::Unknown)
{
    superTimer.start();
}

PlaybackWorker::~PlaybackWorker()
{
    m_listeners.clear();
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

void PlaybackWorker::addListener(PlaybackListener* listener)
{
    QWriteLocker locker(&m_listenersLock);

    if (!m_listeners.contains(listener)) {
        listener->m_worker = this;
        m_listeners << listener;
    }
}

void PlaybackWorker::removeListener(PlaybackListener* listener)
{
    QWriteLocker locker(&m_listenersLock);

    if (m_listeners.contains(listener)) {
        m_listeners.removeAll(listener);
    }
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

    m_counterLock.lockForRead();
    if (frameIndex == m_framesCounter) {
        result = true;
    }
    m_counterLock.unlock();

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
    QList<shared_ptr<StreamInstance>> readInstances;
    QElapsedTimer timer;
    qint64 availableTime;
    qint64 remainingTime = 0;
    qint64 totalTime = 0;
    qint64 averageTime = 0;
    m_framesCounter = 0;
    qint64 offsetTime = 0;

    m_running = true;
    setupListeners();
    openAllInstances();
    timer.start();

    while (m_running)
    {
        // Frames counter
        m_counterLock.lockForWrite();
        m_framesCounter++;
        m_counterLock.unlock();

        // Retrieve instances from which frames has been read
        readInstances = readAllInstances(); // Debug: 20 ms, Release: 10 ms

        // Notify instances 0.02 ms
        if (readInstances.count() > 0) {
            m_running = notifyListeners(readInstances);
        } else {
            m_running = false;
        }

        // Time management: Do I have time?
        availableTime = m_slotTime + offsetTime;
        remainingTime = availableTime - timer.nsecsElapsed();

        if (remainingTime > 0) {
            QThread::currentThread()->usleep(remainingTime / 1000);
        }

        totalTime = timer.nsecsElapsed();
        timer.restart();
        averageTime += totalTime;
        m_fps = 1000000000 / totalTime;
        offsetTime = availableTime - totalTime;
    }

    m_running = false;
    closeAllInstances();

    if (m_framesCounter > 0)
        qDebug() << "Average Time:" << (averageTime / m_framesCounter) / 1000 << "Frame Count:" << m_framesCounter;
}

void PlaybackWorker::stop()
{
    m_running = false;
}

void PlaybackWorker::setupListeners()
{
    if (QThreadPool::globalInstance()->maxThreadCount() < m_listeners.size())
        QThreadPool::globalInstance()->setMaxThreadCount(m_listeners.size());
}

// Devuelve true si hay listeners a los que notificar, false en caso contrario.
inline bool PlaybackWorker::notifyListeners(QList<shared_ptr<StreamInstance> > instances)
{
    // Prepare data
    QHashDataFrames readFrames;

    foreach (shared_ptr<StreamInstance> instance, instances) {
        QList<shared_ptr<DataFrame>> frames = instance->frames();
        foreach (shared_ptr<DataFrame> frame, frames) {
            readFrames.insert(frame->getType(), frame);
        }
    }

    // Notify listeners (Time is measured since this method is called and until the notification is received)
    // signals and slots, debug,   25 fps, Max 29.46 (ms), Min 0.05 (ms), Avg 14.44 (ms)
    // New Approach: Max (ms) 6.08369 Min (ms) 1.02753 Avg (ms) 1.50493
    QReadLocker locker(&m_listenersLock);

    foreach (PlaybackListener* listener, m_listeners) {
        QFuture<void> future = listener->m_future;
        if (future.isFinished()) {
            listener->m_future = QtConcurrent::run(listener, &PlaybackListener::newFrames, readFrames, m_framesCounter);
        }
    }

    return !m_listeners.isEmpty();
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
inline QList<shared_ptr<StreamInstance> > PlaybackWorker::readAllInstances()
{
    QList<shared_ptr<StreamInstance>> instances = m_instances; // implicit sharing
    QList<shared_ptr<StreamInstance>> changedInstances;

    foreach (shared_ptr<StreamInstance> instance, instances)
    {
        if (instance->is_open())
        {
            if (!instance->hasNext() && m_playloop_enabled)
                instance->restart();

            if (instance->hasNext()) {
                instance->readNextFrames();
                changedInstances << instance;
            } else {
                instance->close();
                qDebug() << "Closed";
            }
        }
    }

    return changedInstances;
}

} // End Namespace
