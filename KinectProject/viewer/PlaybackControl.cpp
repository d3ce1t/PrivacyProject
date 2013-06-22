#include "PlaybackControl.h"
#include <QTimer>
#include <QDebug>
#include "types/DataFrame.h"

namespace dai {

PlaybackControl::PlaybackControl()
    : m_worker(this)
{
    m_playloop_enabled = false;
    m_token = 0;
    for (int i=0; i<200; ++i)
        m_usedToken[i] = false;
    m_mutex.lock();
    m_viewers = 0;
    m_mutex.unlock();
}

PlaybackControl::~PlaybackControl()
{
    this->stop();
    m_instances.clear();
}

void PlaybackControl::stop()
{
    QListIterator<StreamInstance*> it(m_instances);

    while (it.hasNext()) {
        StreamInstance* instance = it.next();
        instance->close();
    }
}

void PlaybackControl::play(bool restartAll)
{
    // Open all instances
    QListIterator<StreamInstance*> it(m_instances);

    while (it.hasNext()) {
        StreamInstance* instance = it.next();
        instance->open();
        qDebug() << "Open";
    }

    // Start playback worker
    m_worker.start();
}

void PlaybackControl::doWork()
{
    QListIterator<StreamInstance*> it(m_instances);

    while (it.hasNext()) {
        StreamInstance* instance = it.next();
        if (instance->hasNext()) {
            instance->readNextFrame();
        } else if (m_playloop_enabled) {
            instance->restart();
            instance->readNextFrame();
        } else {
            instance->close();
            qDebug() << "Closed";
        }
    }

    emit newFrameRead();
}

int PlaybackControl::acquire()
{
    QMutexLocker locker(&m_mutex);
    m_viewers++;
    int token = m_token;
    m_usedToken[token] = true;
    m_token = (m_token + 1) % 200;
    return token;
}

void PlaybackControl::release(int token)
{
    QMutexLocker locker(&m_mutex);

    if (m_usedToken[token])
    {
        m_usedToken[token] = false;
        m_viewers--;

        if (m_viewers == 0)
            m_worker.sync();
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
