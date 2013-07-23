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
        QListIterator<shared_ptr<StreamInstance>> it(m_instances);

        while (it.hasNext()) {
            shared_ptr<StreamInstance> instance = it.next();
            if (!instance->is_open()) {
                instance->open();
                std::cerr << "Open" << std::endl;
            }
        }

        // Start playback worker (if already running it does nothing)
        if (m_thread == nullptr) {
            m_thread = new QThread;
            m_worker = new PlaybackWorker(this);
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
    QListIterator<shared_ptr<StreamInstance> > it(m_instances);

    // m_thread and m_worker are connected to deleteLater, so I only
    // mark they as null
    m_thread = nullptr;
    m_worker = nullptr;

    // Close all opened instances
    while (it.hasNext()) {
        shared_ptr<StreamInstance> instance = it.next();
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
QList<shared_ptr<StreamInstance> > PlaybackControl::readAllInstances()
{
    QList<shared_ptr<StreamInstance>> instances = m_instances; // implicit sharing
    QList<shared_ptr<StreamInstance>> changedInstances;

    // qDebug() << "Worker" << QThread::currentThreadId();

    foreach (shared_ptr<StreamInstance> instance, instances)
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
void PlaybackControl::notifyListeners(QList<shared_ptr<StreamInstance>> changedInstances)
{
    QMultiHash<StreamInstance*, PlaybackListener*> instanceToListenerMap = m_instanceToListenerMap;
    QHash<PlaybackListener*, QList<shared_ptr<DataFrame>>> sendResult;

    foreach (shared_ptr<StreamInstance> instance, changedInstances)
    {
        QList<PlaybackListener*> listenerList = instanceToListenerMap.values(instance.get());
        shared_ptr<DataFrame> frame = instance->frame();

        foreach (PlaybackListener* listener, listenerList)
        {
           QList<shared_ptr<DataFrame>> listFrames;
           listFrames = sendResult.value(listener);
           listFrames << frame;
           sendResult.insert(listener, listFrames);
        }
    }

    foreach (PlaybackListener* listener, sendResult.keys())
        listener->onNewFrame(sendResult.value(listener));
}

void PlaybackControl::addListener(PlaybackListener* listener, shared_ptr<StreamInstance> instance)
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

void PlaybackControl::removeListener(PlaybackListener *listener, StreamInstance::StreamType type)
{
    shared_ptr<StreamInstance> instance = nullptr;

    m_lockListeners.lock();

    if (m_listeners.contains(listener))
    {
        QList<shared_ptr<StreamInstance>> instanceList = m_listenerToInstanceMap.values(listener);
        QListIterator<shared_ptr<StreamInstance>> it(instanceList);

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

// remove from m_listeners, m_listenerToInstanceMap and m_instanceToListenerMap
void PlaybackControl::removeListener(PlaybackListener* listener, shared_ptr<StreamInstance> instance)
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
        QList<shared_ptr<StreamInstance>> instanceList = m_listenerToInstanceMap.values(listener);

        foreach (shared_ptr<StreamInstance> instance, instanceList)
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

float PlaybackControl::getFPS() const
{
    return m_worker->getFPS();
}

void PlaybackControl::addInstance(shared_ptr<StreamInstance> instance)
{
    if (!m_instances.contains(instance))
        m_instances << instance;
}

void PlaybackControl::enablePlayLoop(bool value)
{
    m_playloop_enabled = value;
}

} // End namespace
