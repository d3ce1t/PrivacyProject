#include "PlaybackControl.h"
#include <iostream>
#include <QDebug>

namespace dai {

PlaybackControl::PlaybackControl()
{
    m_worker = new PlaybackWorker(this, m_instances);
    m_worker->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_worker, &PlaybackWorker::onNewFrames, this, &PlaybackControl::onNewFrames);
    connect(m_worker, &PlaybackWorker::onStop, this, &PlaybackControl::onStop);
    m_workerThread.start();
}

PlaybackControl::~PlaybackControl()
{
    stop();
    m_workerThread.quit();
    m_workerThread.wait();
    m_instances.clear();
    std::cerr << "PlaybackControl::~PlaybackControl()" << std::endl;
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
    std::cerr << "PlaybackControl::stop()" << std::endl;
}

void PlaybackControl::setFPS(float fps)
{
    m_worker->setFPS(fps);
}

float PlaybackControl::getFPS() const
{
    return m_worker->getFPS();
}

bool PlaybackControl::isValidFrame(qint64 frameIndex) const
{
    return m_worker->isValidFrame(frameIndex);
}

void PlaybackControl::addInstance(shared_ptr<BaseInstance> instance)
{
    if (!m_instances.contains(instance))
        m_instances << instance;
}

void PlaybackControl::removeInstance(shared_ptr<BaseInstance> instance)
{
    if (m_instances.contains(instance))
        m_instances.removeAll(instance);
}

void PlaybackControl::enablePlayLoop(bool value)
{
    m_worker->enablePlayLoop(value);
}

} // End namespace
