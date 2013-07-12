#include "PlaybackControl.h"
#include <QTimer>
#include "types/DataFrame.h"
#include "PlaybackWorker.h"
#include <iostream>

namespace dai {

PlaybackControl::PlaybackListener::PlaybackListener()
{
    m_playback = NULL;
}

PlaybackControl::PlaybackListener::~PlaybackListener()
{
    if (m_playback != NULL) {
        m_playback->release(this);
        m_playback->removeListener(this);
        m_playback = NULL;
    }
}

void PlaybackControl::PlaybackListener::setPlayback(PlaybackControl* playback)
{
    m_playback = playback;
}

void PlaybackControl::PlaybackListener::releasePlayback()
{
    if (m_playback != NULL) {
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
    m_worker = NULL;
    m_clearInstances = true;
    m_restartAfterStop = false;
}

PlaybackControl::~PlaybackControl()
{
    this->stop();

    // Clear listeners
    removeAllListeners();

    // Clear instances
    foreach (StreamInstance* instance, m_instances) {
        delete instance;
    }
    m_instances.clear();

    if (m_worker != NULL) {
        delete m_worker;
        m_worker = NULL;
    }

    std::cerr << "PlaybackControl::~PlaybackControl()" << std::endl;
}

void PlaybackControl::stop()
{
    if (m_worker != NULL) {
        m_worker->stop();
    }
}

void PlaybackControl::stopAsync()
{
    std::cerr << "PlaybackControl::stopAsync()" << std::endl;
    QListIterator<StreamInstance*> it(m_instances);

    if (m_worker != NULL) {
        m_worker->wait(5000);
        delete m_worker;
        m_worker = NULL;
    }

    while (it.hasNext()) {
        StreamInstance* instance = it.next();
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
    if (m_worker != NULL && m_worker->isRunning() && restartAll) {
        m_restartAfterStop = true;
        stop();
    } else {
        // Open all instances
        QListIterator<StreamInstance*> it(m_instances);

        while (it.hasNext()) {
            StreamInstance* instance = it.next();
            if (!instance->is_open()) {
                instance->open();
                std::cerr << "Open" << std::endl;
            }
        }

        // Start playback worker (if already running it does nothing)
        if (m_worker == NULL) {
            m_worker = new PlaybackWorker(this);
        }

        m_worker->start();
    }
}

bool PlaybackControl::doWork()
{
    QList<StreamInstance*> instances = m_instances; // implicit sharing
    QList<StreamInstance*> notChangedInstances;
    bool frameAvailable = false;

    foreach (StreamInstance* instance, instances)
    {
        if (instance->is_open())
        {
            // Not suscribers
            if (!m_listenersAux.contains(instance)) {
                instance->close();
                notChangedInstances << instance;
                m_instances.removeOne(instance);
                if (m_clearInstances)
                    delete instance;
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

void PlaybackControl::addNewFrameListener(PlaybackListener* listener, StreamInstance* instance)
{
    QMutexLocker locker(&m_lockListeners);
    QList<StreamInstance*>* instanceList = NULL;

    // Listeners
    if (!m_listeners.contains(listener)) {
        instanceList = new QList<StreamInstance*>;
        m_listeners.insert(listener, instanceList);
    } else {
        instanceList = m_listeners.value(listener);
    }

    *instanceList << instance;

    // Instances
    QList<PlaybackListener*>* listenerList = NULL;

    if (!m_listenersAux.contains(instance)) {
        listenerList = new QList<PlaybackListener*>;
        m_listenersAux.insert(instance, listenerList);
    } else {
        listenerList = m_listenersAux.value(instance);
    }

    *listenerList << listener;

    // Set Playback
    listener->setPlayback(this);
}

void PlaybackControl::removeListener(PlaybackListener* listener, StreamInstance::StreamType type)
{
    StreamInstance* instance = NULL;

    m_lockListeners.lock();
    if (m_listeners.contains(listener)) {
        QList<StreamInstance*>* instanceList = m_listeners.value(listener);
        QListIterator<StreamInstance*> it(*instanceList);

        while (it.hasNext() && instance == NULL) {
            StreamInstance* tmpInstance = it.next();
            if (tmpInstance->getType() == type) {
                instance = tmpInstance;
            }
        }
    }
    m_lockListeners.unlock();

    if (instance != NULL)
        removeListener(listener, instance);
}

void PlaybackControl::removeListener(PlaybackListener* listener, StreamInstance* instance)
{
    QMutexLocker locker(&m_lockListeners);

    if (m_listeners.contains(listener)) {
        QList<StreamInstance*>* instanceList = m_listeners.value(listener);
        instanceList->removeOne(instance);
    }

    if (m_listenersAux.contains(instance)) {
        QList<PlaybackListener*>* listenerList = m_listenersAux.value(instance);
        if (listenerList->contains(listener)) {
            listenerList->removeOne(listener);
        }
        if (listenerList->isEmpty()) {
            m_listenersAux.remove(instance);
        }
    }
}

void PlaybackControl::removeListener(PlaybackListener* listener)
{
    QMutexLocker locker(&m_lockListeners);

    if (m_listeners.contains(listener)) {
        QList<StreamInstance*>* instanceList = m_listeners.value(listener);
        instanceList->clear();
        delete instanceList;
        m_listeners.remove(listener);
    }

    QHashIterator<StreamInstance*, QList<PlaybackListener*>*> it(m_listenersAux);
    QList<StreamInstance*> removeInstances;

    while (it.hasNext()) {
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

    foreach (QList<StreamInstance*>* instanceList, m_listeners.values()) {
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

void PlaybackControl::notifySuscribersOnNewFrames(QList<StreamInstance*> notChangedInstances)
{
    QMutexLocker locker(&m_lockListeners);

    foreach (PlaybackListener* listener, m_listeners.keys())
    {
        QList<StreamInstance*>* instanceList = m_listeners.value(listener);
        foreach (StreamInstance* instance, notChangedInstances) {
            instanceList->removeOne(instance);
        }

        QList<DataFrame*> frameList;
        foreach(StreamInstance* instance, *instanceList) {
            frameList << &instance->frame();
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

void PlaybackControl::addInstance(StreamInstance* instance)
{
    m_instances << instance;
}

void PlaybackControl::enablePlayLoop(bool value)
{
    m_playloop_enabled = value;
}

void PlaybackControl::setClearInstances(bool value)
{
    m_clearInstances = value;
}

} // End namespace
