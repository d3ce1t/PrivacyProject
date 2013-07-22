#include "PlaybackControl.h"
#include <QTimer>
#include "types/DataFrame.h"
#include <iostream>

namespace dai {

PlaybackControl::PlaybackControl()
{
    m_playloop_enabled = false;
    m_restartAfterStop = false;
    m_worker = nullptr;
}

PlaybackControl::~PlaybackControl()
{
    this->stop();

    // Clear listeners
    removeAllListeners();

    m_instances.clear();

    if (m_worker != nullptr) {
        delete m_worker;
        m_worker = nullptr;
    }

    std::cerr << "PlaybackControl::~PlaybackControl()" << std::endl;
}

void PlaybackControl::stop()
{
    if (m_worker != nullptr) {
        m_worker->stop();
    }
}

void PlaybackControl::stopAsync()
{
    std::cerr << "PlaybackControl::stopAsync()" << std::endl;
    QListIterator<shared_ptr<StreamInstance> > it(m_instances);

    if (m_worker != nullptr) {
        m_worker->wait(5000);
        delete m_worker;
        m_worker = nullptr;
    }

    while (it.hasNext()) {
        shared_ptr<StreamInstance> instance = it.next();
        if (instance->is_open()) {
            instance->close();
            std::cerr << "Close" << std::endl;
        }
    }

    notifySuscribersOnStop();

    emit onPlaybackFinished(this);

    if (m_restartAfterStop) {
        m_restartAfterStop = false;
        play();
    }
}

void PlaybackControl::play(bool restartAll)
{
    if (m_worker != nullptr && m_worker->isRunning() && restartAll) {
        m_restartAfterStop = true;
        stop();
    } else {
        // Open all instances
        QListIterator<shared_ptr<StreamInstance>> it(m_instances);

        while (it.hasNext()) {
            shared_ptr<StreamInstance> instance = it.next();
            if (!instance->is_open()) {
                instance->open();
                std::cerr << "Open" << std::endl;
            }
        }

        // Start playback worker (if already running it does nothing)
        if (m_worker == nullptr) {
            m_worker = new PlaybackWorker(this);
        }

        m_worker->start();
    }
}

bool PlaybackControl::readAllInstances()
{
    QList<shared_ptr<StreamInstance>> instances = m_instances; // implicit sharing
    QList<shared_ptr<StreamInstance>> notChangedInstances;
    bool frameAvailable = false;

    foreach (shared_ptr<StreamInstance> instance, instances)
    {
        if (instance->is_open())
        {
            if (hasSuscribers(instance))
            {
                if (!instance->hasNext() && m_playloop_enabled)
                    instance->restart();

                if (instance->hasNext()) {
                    instance->readNextFrame();
                    frameAvailable = true;
                } else {
                    instance->close();
                    notChangedInstances << instance;
                    std::cerr << "Closed" << std::endl;
                }
            }
            // Not suscribers
            else {
                instance->close();
                notChangedInstances << instance;
                m_instances.removeOne(instance);
                std::cerr << "Closed (not suscribers)" << std::endl;

            }
        }
    }

    if (frameAvailable)
        notifySuscribersOnNewFrames(notChangedInstances);

    return frameAvailable;
}

bool PlaybackControl::hasSuscribers(shared_ptr<StreamInstance> instance)
{
    bool result = false;

    foreach (QList<shared_ptr<StreamInstance>>* instanceList, m_listeners.values()) {
        if (instanceList->contains(instance)) {
            result = true;
            break;
        }
    }

    return result;
}

void PlaybackControl::addListener(PlaybackListener* listener, shared_ptr<StreamInstance> instance)
{
    QMutexLocker locker(&m_lockListeners);
    QList<shared_ptr<StreamInstance> >* instanceList = nullptr;

    // Listeners
    if (!m_listeners.contains(listener)) {
        instanceList = new QList<shared_ptr<StreamInstance> >;
        m_listeners.insert(listener, instanceList);
    } else {
        instanceList = m_listeners.value(listener);
    }

    *instanceList << instance;

    // Set Playback
    listener->setPlayback(this);
}

void PlaybackControl::removeListener(PlaybackListener *listener, StreamInstance::StreamType type)
{
    shared_ptr<StreamInstance> instance;

    m_lockListeners.lock();

    if (m_listeners.contains(listener))
    {
        QList<shared_ptr<StreamInstance>>* instanceList = m_listeners.value(listener);
        QListIterator<shared_ptr<StreamInstance>> it(*instanceList);

        while (it.hasNext() && !instance) {
            shared_ptr<StreamInstance> tmpInstance = it.next();
            if (tmpInstance->getType() == type) {
                instance = tmpInstance;
            }
        }
    }
    m_lockListeners.unlock();

    if (instance)
        removeListener(listener, instance);
}

void PlaybackControl::removeListener(PlaybackListener* listener, shared_ptr<StreamInstance> instance)
{
    QMutexLocker locker(&m_lockListeners);

    if (m_listeners.contains(listener)) {
        QList<shared_ptr<StreamInstance>>* instanceList = m_listeners.value(listener);
        instanceList->removeOne(instance);
        if (instanceList->isEmpty()) {
            delete instanceList;
            m_listeners.remove(listener);
        }
    }
}

void PlaybackControl::removeListener(PlaybackListener* listener)
{
    QMutexLocker locker(&m_lockListeners);

    if (m_listeners.contains(listener)) {
        QList<shared_ptr<StreamInstance>>* instanceList = m_listeners.value(listener);
        instanceList->clear();
        delete instanceList;
        m_listeners.remove(listener);
    }
}

void PlaybackControl::removeAllListeners()
{
    QMutexLocker locker(&m_lockListeners);

    foreach (QList<shared_ptr<StreamInstance>>* instanceList, m_listeners.values()) {
        instanceList->clear();
        delete instanceList;
    }

    m_listeners.clear();
}

void PlaybackControl::notifySuscribersOnNewFrames(QList<shared_ptr<StreamInstance>> notChangedInstances)
{
    QMutexLocker locker(&m_lockListeners);

    foreach (PlaybackListener* listener, m_listeners.keys())
    {
        QList<shared_ptr<StreamInstance>>* instanceList = m_listeners.value(listener);
        foreach (shared_ptr<StreamInstance> instance, notChangedInstances) {
            instanceList->removeOne(instance);
        }

        QList<shared_ptr<DataFrame>> frameList;
        foreach(shared_ptr<StreamInstance> instance, *instanceList) {
            frameList << instance->frame();
        }

        QMetaObject::invokeMethod(listener, "manageFrames",
                                      Qt::AutoConnection,
                                      Q_ARG(QList<shared_ptr<DataFrame>>, frameList));
    }
}

void PlaybackControl::notifySuscribersOnStop()
{
    QMutexLocker locker(&m_lockListeners);

    foreach (PlaybackListener* listener, m_listeners.keys())
    {
        listener->onPlaybackStop();
    }
}

float PlaybackControl::getFPS() const
{
    return m_worker->getFPS();
}

void PlaybackControl::addInstance(shared_ptr<StreamInstance> instance)
{
    m_instances << instance;
}

void PlaybackControl::enablePlayLoop(bool value)
{
    m_playloop_enabled = value;
}

} // End namespace
