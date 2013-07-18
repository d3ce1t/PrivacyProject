#include "PlaybackControl.h"
#include <QTimer>
#include "types/DataFrame.h"
#include "PlaybackWorker.h"
#include <iostream>

namespace dai {

PlaybackControl::PlaybackListener::PlaybackListener()
{
    m_playback = nullptr;
}

PlaybackControl::PlaybackListener::~PlaybackListener()
{
    if (m_playback != nullptr) {
        m_playback->release(this);
        m_playback->removeListener(this);
        m_playback = nullptr;
    }
}

void PlaybackControl::PlaybackListener::setPlayback(PlaybackControl* playback)
{
    m_playback = playback;
}

void PlaybackControl::PlaybackListener::releasePlayback()
{
    if (m_playback != nullptr) {
        m_playback->release(this);
    }
}

PlaybackControl* PlaybackControl::PlaybackListener::playback()
{
    return m_playback;
}

PlaybackControl::PlaybackControl()
{
    m_playloop_enabled = false;
    m_worker = nullptr;
    m_restartAfterStop = false;
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

bool PlaybackControl::doWork()
{
    QList<shared_ptr<StreamInstance>> instances = m_instances; // implicit sharing
    QList<shared_ptr<StreamInstance>> notChangedInstances;
    bool frameAvailable = false;

    foreach (shared_ptr<StreamInstance> instance, instances)
    {
        if (instance->is_open())
        {
            // Not suscribers
            if (!m_listenersAux.contains(instance.get())) {
                instance->close();
                notChangedInstances << instance;
                m_instances.removeOne(instance);
                /*if (m_clearInstances)
                    delete instance;*/
                std::cerr << "Closed (not suscribers)" << std::endl;
            }
            else {
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
        }
    }

    if (frameAvailable)
        notifySuscribersOnNewFrames(notChangedInstances);

    return frameAvailable;
}

void PlaybackControl::release(PlaybackListener *caller)
{
    m_worker->release(caller);
}

void PlaybackControl::addNewFrameListener(PlaybackListener* listener, shared_ptr<StreamInstance> instance)
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

    // Instances
    QList<PlaybackListener*>* listenerList = nullptr;

    if (!m_listenersAux.contains(instance.get())) {
        listenerList = new QList<PlaybackListener*>;
        m_listenersAux.insert(instance.get(), listenerList);
    } else {
        listenerList = m_listenersAux.value(instance.get());
    }

    *listenerList << listener;

    // Set Playback
    listener->setPlayback(this);
}

void PlaybackControl::removeListener(PlaybackListener *listener, StreamInstance::StreamType type)
{
    shared_ptr<StreamInstance> instance;

    m_lockListeners.lock();

    if (m_listeners.contains(listener)) {
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
    }

    if (m_listenersAux.contains(instance.get())) {
        QList<PlaybackListener*>* listenerList = m_listenersAux.value(instance.get());
        if (listenerList->contains(listener)) {
            listenerList->removeOne(listener);
        }
        if (listenerList->isEmpty()) {
            m_listenersAux.remove(instance.get());
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

    QHashIterator<StreamInstance*, QList<PlaybackListener*>*> it(m_listenersAux);
    QList<StreamInstance*> removeInstances;

    while (it.hasNext())
    {
        it.next();
        StreamInstance* instance = it.key();
        QList<PlaybackListener*>* listenerList = it.value();

        if (listenerList->contains(listener)) {
            listenerList->removeOne(listener);
        }

        if (listenerList->isEmpty()) {
            removeInstances << instance;
            delete listenerList;
        }
    }

    foreach (StreamInstance* instance, removeInstances) {
        m_listenersAux.remove(instance);
    }

    removeInstances.clear();
}

void PlaybackControl::removeAllListeners()
{
    QMutexLocker locker(&m_lockListeners);

    foreach (QList<shared_ptr<StreamInstance>>* instanceList, m_listeners.values()) {
        instanceList->clear();
        delete instanceList;
    }

    foreach (QList<PlaybackListener*>* listenerList, m_listenersAux.values()) {
        listenerList->clear();
        delete listenerList;
    }

    m_listeners.clear();
    m_listenersAux.clear();
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

        m_worker->acquire(listener);
        listener->onNewFrame(frameList);
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
