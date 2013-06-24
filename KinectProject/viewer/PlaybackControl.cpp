#include "PlaybackControl.h"
#include <QTimer>
#include <QDebug>
#include "types/DataFrame.h"
#include <cstdlib>

namespace dai {

PlaybackControl::PlaybackControl()
    : m_worker(this)
{
    srand(time(NULL));
    m_playloop_enabled = false;
    m_lockToken.lock();
    m_viewers = 0;
    m_lockToken.unlock();
}

PlaybackControl::~PlaybackControl()
{
    this->stop();
    m_instances.clear();
}

void PlaybackControl::stop()
{
    QListIterator<StreamInstance*> it(m_instances);
    m_worker.stop();
    m_worker.wait(5000);

    while (it.hasNext()) {
        StreamInstance* instance = it.next();
        instance->close();
        qDebug() << "Close";
    }

    qDebug() << "PlaybackControl::stop()";
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
    m_worker.start();
}

void PlaybackControl::doWork()
{
    QList<StreamInstance*> instances = m_instances; // implicit sharing
    QListIterator<StreamInstance*> it(instances);
    QList<StreamInstance*> instanceList;
    bool frameAvailable = false;

    while (it.hasNext())
    {
        StreamInstance* instance = it.next();

        if (instance->is_open())
        {
            if (!instance->hasNext() && m_playloop_enabled)
                instance->restart();

            if (instance->hasNext()) {
                instance->readNextFrame();
                frameAvailable = true;
            } else {
                instance->close();
                instanceList << instance;
                qDebug() << "Closed";
            }
        }
    }

    if (frameAvailable) {
        notifySuscribers(instanceList);

    } else {
        m_worker.stop();
    }
}

int PlaybackControl::acquire(QObject* caller)
{
    QMutexLocker locker(&m_lockToken);
    m_viewers++;
    int token = rand();
    m_usedTokens.insert(caller, token);
    return token;
}

void PlaybackControl::release(QObject* caller, int token)
{
    QMutexLocker locker(&m_lockToken);
    if (m_usedTokens.value(caller) == token) {
        m_usedTokens.remove(caller);
        m_viewers--;
        if (m_viewers == 0)
            m_worker.sync();
    }
}

void PlaybackControl::addNewFrameListener(PlaybackListener* listener, StreamInstance* instance)
{
    QMutexLocker locker(&m_lockListeners);
    QList<StreamInstance*>* instanceList = NULL;

    if (!m_listenersAux.contains(listener)) {
        instanceList = new QList<StreamInstance*>;
        m_listenersAux.insert(listener, instanceList);
    } else {
        instanceList = m_listenersAux.value(listener);
    }

    *instanceList << instance;
}

void PlaybackControl::removeListener(PlaybackListener* listener, StreamInstance* instance)
{
    QMutexLocker locker(&m_lockListeners);

    if (m_listenersAux.contains(listener)) {
        QList<StreamInstance*>* instanceList = m_listenersAux.value(listener);
        instanceList->removeOne(instance);
    }
}

void PlaybackControl::removeAllListeners(PlaybackListener* listener)
{
    QMutexLocker locker(&m_lockListeners);
    if (m_listenersAux.contains(listener)) {
        QList<StreamInstance*>* instanceList = m_listenersAux.value(listener);
        instanceList->clear();
        delete instanceList;
        m_listenersAux.remove(listener);
    }
}

void PlaybackControl::notifySuscribers(QList<StreamInstance*> notChangedInstances)
{
    QMutexLocker locker(&m_lockListeners);

    foreach (PlaybackListener* listener, m_listenersAux.keys())
    {
        QList<StreamInstance*>* instanceList = m_listenersAux.value(listener);
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
    return m_worker.getFPS();
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
