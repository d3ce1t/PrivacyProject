#include "PlaybackControl.h"
#include <QTimer>
#include <QDebug>
#include "types/DataFrame.h"
#include <cstdlib>

namespace dai {

PlaybackControl::PlaybackControl()
{
    srand(time(NULL));
    m_playloop_enabled = false;
    m_lockViewers.lock();
    m_viewers = 0;
    m_lockViewers.unlock();
    m_worker = NULL;
}

PlaybackControl::~PlaybackControl()
{
    this->stop();
    foreach (StreamInstance* instance, m_instances) {
        delete instance;
    }
    m_instances.clear();
    m_listeners.clear();
    m_listenersAux.clear();
    m_worker = NULL;
}

void PlaybackControl::stop()
{
    QListIterator<StreamInstance*> it(m_instances);
    m_worker->stop();
    m_worker->wait(5000);
    delete m_worker;
    m_worker = NULL;

    while (it.hasNext()) {
        StreamInstance* instance = it.next();
        if (instance->is_open()) {
            instance->close();
            qDebug() << "Close";
        }
    }
}

void PlaybackControl::play(bool restartAll)
{
    if (restartAll)
        stop();

    // Open all instances
    QListIterator<StreamInstance*> it(m_instances);

    while (it.hasNext()) {
        StreamInstance* instance = it.next();
        if (!instance->is_open()) {
            instance->open();
            qDebug() << "Open";
        }
    }

    // Start playback worker (if already running it does nothing)
    if (m_worker == NULL) {
        m_worker = new PlaybackWorker(this);
    }

    m_worker->start();
}

void PlaybackControl::doWork()
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
                qDebug() << "Closed (not suscribers)";
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
                    qDebug() << "Closed";
                }
            }
        }
    }

    if (frameAvailable) {
        notifySuscribers(notChangedInstances);
    } else {
        this->stop();
    }
}

int PlaybackControl::acquire(QObject* caller)
{
    QMutexLocker locker(&m_lockViewers);
    m_viewers++;
    int token = rand();
    m_usedTokens.insert(caller, token);
    return token;
}

void PlaybackControl::release(QObject* caller, int token)
{
    QMutexLocker locker(&m_lockViewers);
    if (m_usedTokens.value(caller) == token) {
        m_usedTokens.remove(caller);
        m_viewers--;
        if (m_viewers == 0)
            m_worker->sync();
    }
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

void PlaybackControl::removeAllListeners(PlaybackListener* listener)
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
        }
    }

    foreach (StreamInstance* instance, removeInstances) {
        m_listenersAux.remove(instance);
    }

    removeInstances.clear();
}

void PlaybackControl::notifySuscribers(QList<StreamInstance*> notChangedInstances)
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

        listener->onNewFrame(frameList);
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

} // End namespace
