#include "PlaybackControl.h"
#include "PlaybackListener.h"
#include "PlaybackWorker.h"
#include <QDebug>

namespace dai {

PlaybackControl::PlaybackControl()
{
    m_worker = new PlaybackWorker;
    m_worker->moveToThread(&m_workerThread);
    QObject::connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_workerThread.start();
}

PlaybackControl::~PlaybackControl()
{
    stop();
    m_workerThread.quit();
    m_workerThread.wait();
    qDebug() << "PlaybackControl::~PlaybackControl()";
}

void PlaybackControl::play(bool restartAll)
{
    if (restartAll) {
        stop();
        QMetaObject::invokeMethod(m_worker, "run", Qt::AutoConnection);
    } else {
        QMetaObject::invokeMethod(m_worker, "run", Qt::AutoConnection);
    }
}

void PlaybackControl::stop()
{
    m_worker->stop();
    qDebug() << "PlaybackControl::stop()";
}

void PlaybackControl::setFPS(float fps)
{
    m_worker->setFPS(fps);
}

void PlaybackControl::addListener(PlaybackListener *listener)
{
    m_worker->addListener(listener);
}

void PlaybackControl::removeListener(PlaybackListener *listener)
{
    m_worker->removeListener(listener);
}

void PlaybackControl::addInstance(shared_ptr<StreamInstance> instance)
{
    m_worker->addInstance(instance);
}

void PlaybackControl::removeInstance(shared_ptr<StreamInstance> instance)
{
    m_worker->removeInstance(instance);
}

void PlaybackControl::clearInstances()
{
    m_worker->clearInstances();
}

void PlaybackControl::enablePlayLoop(bool value)
{
    m_worker->enablePlayLoop(value);
}

} // End namespace
