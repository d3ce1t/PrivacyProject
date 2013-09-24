#include "PlaybackControl.h"
#include <QTimer>
#include "types/DataFrame.h"
#include <iostream>
#include <QDebug>

namespace dai {

PlaybackControl::PlaybackControl()
{
    m_playloop_enabled = false;
    m_restartAfterStop = false;
    m_worker = nullptr;
    m_thread = nullptr;
    m_fps = 25;
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

    if (m_thread != nullptr) {
        delete m_thread;
        m_thread = nullptr;
    }

    std::cerr << "PlaybackControl::~PlaybackControl()" << std::endl;
}

void PlaybackControl::play(bool restartAll)
{
    if (m_thread != nullptr && m_thread->isRunning() && restartAll) {
        m_restartAfterStop = true;
        stop();
    }
    else {
        // Open all instances
        QListIterator<shared_ptr<BaseInstance>> it(m_instances);

        while (it.hasNext()) {
            shared_ptr<BaseInstance> instance = it.next();
            if (!instance->is_open()) {
                try {
                    instance->open();
                    std::cerr << "Open" << std::endl;
                }
                catch (CannotOpenInstanceException ex) {
                    removeInstance(instance);
                    throw ex;
                }
            }
        }

        // Start playback worker (if already running it does nothing)
        if (m_thread == nullptr) {
            m_thread = new QThread;
            m_worker = new PlaybackWorker(this);
            m_worker->setFPS(m_fps);
            m_worker->moveToThread(m_thread);
            QObject::connect(m_thread, SIGNAL(started()), m_worker, SLOT(run()));
            QObject::connect(m_worker, SIGNAL(finished()), m_thread, SLOT(quit()));
            QObject::connect(m_worker, SIGNAL(finished()), this, SLOT(stopAsync()));
            QObject::connect(m_worker, SIGNAL(finished()), m_worker, SLOT(deleteLater()));
            QObject::connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
        }

        m_thread->start();
    }
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
    QListIterator<shared_ptr<BaseInstance> > it(m_instances);

    // m_thread and m_worker are connected to deleteLater, so I only
    // mark they as null
    m_thread = nullptr;
    m_worker = nullptr;

    // Close all opened instances
    while (it.hasNext()) {
        auto instance = it.next();
        if (instance->is_open()) {
            instance->close();
            std::cerr << "Close" << std::endl;
        }
    }

    if (m_restartAfterStop) {
        m_restartAfterStop = false;
        play();
    }
    else {
        // Notify suscribers
        notifySuscribersOnStop();
        emit onPlaybackFinished(this);
    }
}

// Called from Worker thread
QList<shared_ptr<BaseInstance> > PlaybackControl::readAllInstances()
{
    QList<shared_ptr<BaseInstance>> instances = m_instances; // implicit sharing
    QList<shared_ptr<BaseInstance>> changedInstances;

    foreach (shared_ptr<BaseInstance> instance, instances)
    {
        if (instance->is_open())
        {
            // Has Suscribers
            if ( m_instanceToListenerMap.contains(instance.get()) )
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
            // Not suscribers
            else {
                instance->close();
                m_instances.removeOne(instance);
                std::cerr << "Closed (not suscribers)" << std::endl;
            }
        }
    }

    return changedInstances;
}

// Called from Notifier thread
void PlaybackControl::notifyListeners(QList<shared_ptr<BaseInstance> > changedInstances)
{
    auto instanceToListenerMap = m_instanceToListenerMap;
    QHash<PlaybackListener*, QHashDataFrames> sendResult;

    foreach (shared_ptr<BaseInstance> instance, changedInstances)
    {
        QList<PlaybackListener*> listenerList = instanceToListenerMap.values(instance.get());
        shared_ptr<DataFrame> frame = instance->frame();

        foreach (PlaybackListener* listener, listenerList)
        {
           QHashDataFrames hashFrames = sendResult.value(listener); // gets a copy
           hashFrames.insert(frame->getType(), frame);
           sendResult.insert(listener, hashFrames);
        }
    }

    foreach (PlaybackListener* listener, sendResult.keys())
        if (listener != nullptr)
            listener->onNewFrame(sendResult.value(listener));
}

void PlaybackControl::addListener(PlaybackListener* listener, shared_ptr<BaseInstance> instance)
{
    QMutexLocker locker(&m_lockListeners);

    // Set Playback
    if (listener->playback() != nullptr && listener->playback() != this) {
        std::cerr << "PlaybackListener already has a playback attached" << endl;
    } else {
        listener->setPlayback(this);
    }

    if (!m_listeners.contains(listener)) {
        m_listeners << listener;
    }

    m_listenerToInstanceMap.insert(listener, instance);
    m_instanceToListenerMap.insert(instance.get(), listener);
}

void PlaybackControl::removeListener(PlaybackListener *listener, InstanceType type)
{
    shared_ptr<BaseInstance> instance = nullptr;

    m_lockListeners.lock();

    if (m_listeners.contains(listener))
    {
        QList<shared_ptr<BaseInstance>> instanceList = m_listenerToInstanceMap.values(listener);
        QListIterator<shared_ptr<BaseInstance>> it(instanceList);

        while (it.hasNext() && !instance) {
            shared_ptr<BaseInstance> tmpInstance = it.next();
            if (tmpInstance->getType() == type) {
                instance = tmpInstance;
            }
        }
    }
    m_lockListeners.unlock();

    if (instance)
        removeListener(listener, instance);
}

// remove from m_listeners, m_listenerToInstanceMap and m_instanceToListenerMap
void PlaybackControl::removeListener(PlaybackListener* listener, shared_ptr<BaseInstance> instance)
{
    QMutexLocker locker(&m_lockListeners);

    if (m_listeners.contains(listener))
    {
        // remove from m_listenerToInstanceMap
        m_listenerToInstanceMap.remove(listener, instance);

        if (!m_listenerToInstanceMap.contains(listener)) {
            m_listeners.removeOne(listener);
        }

        // remove from m_instanceToListenerMap
        m_instanceToListenerMap.remove(instance.get(), listener);

        /*if (!m_instanceToListenerMap.contains(instance.get())) {
            m_instances.removeOne(instance);
        }*/
    }
}

void PlaybackControl::removeListener(PlaybackListener *listener)
{
    QMutexLocker locker(&m_lockListeners);

    if (m_listeners.contains(listener))
    {
        QList<shared_ptr<BaseInstance>> instanceList = m_listenerToInstanceMap.values(listener);

        foreach (shared_ptr<BaseInstance> instance, instanceList)
        {
            m_instanceToListenerMap.remove(instance.get(), listener);

            /*if (!m_instanceToListenerMap.contains(instance.get())) {
                m_instances.removeAll(instance);
            }*/
        }

        m_listenerToInstanceMap.remove(listener);
        m_listeners.removeOne(listener);
    }
}

void PlaybackControl::removeAllListeners()
{
    QMutexLocker locker(&m_lockListeners);
    m_listenerToInstanceMap.clear();
    m_instanceToListenerMap.clear();
    m_listeners.clear();
    //m_instances.clear();
}

void PlaybackControl::notifySuscribersOnStop()
{
    QMutexLocker locker(&m_lockListeners);

    foreach (PlaybackListener* listener, m_listeners)
    {
        listener->onPlaybackStop();
    }
}

void PlaybackControl::setFPS(float fps)
{
    m_fps = fps;
}

float PlaybackControl::getFPS() const
{
    return m_worker ? m_worker->getFPS() : 0.0;
}

void PlaybackControl::addInstance(shared_ptr<BaseInstance> instance)
{
    if (!m_instances.contains(instance))
        m_instances << instance;
}

void PlaybackControl::removeInstance(shared_ptr<BaseInstance> instance)
{
    QList<PlaybackListener*> listeners = m_instanceToListenerMap.values(instance.get());

    foreach (PlaybackListener* listener, listeners) {
        removeListener(listener, instance);
    }

    if (m_instances.contains(instance))
        m_instances.removeAll(instance);
}

void PlaybackControl::enablePlayLoop(bool value)
{
    m_playloop_enabled = value;
}

} // End namespace
