#include "PlaybackWorker.h"
#include "PlaybackControl.h"
#include <QThread>
#include <QElapsedTimer>
#include <iostream>

namespace dai {

PlaybackWorker::PlaybackWorker(PlaybackControl* parent)
    : m_sleepTime(100)
{
    m_parent = parent;
    m_running = false;
}

void PlaybackWorker::initialise()
{
    m_thread = new QThread;
    m_notifier = new PlaybackNotifier(m_parent);
    m_notifier->moveToThread(m_thread);
    //QObject::connect(m_thread, SIGNAL(started()), m_notifier, SLOT(run()));
    //QObject::connect(m_notifier, SIGNAL(finished()), m_thread, SLOT(quit()));
    //QObject::connect(m_notifier, SIGNAL(finished()), m_notifier, SLOT(deleteLater()));
    QObject::connect(m_thread, SIGNAL(finished()), m_thread, SLOT(deleteLater()));
    QObject::connect(this, SIGNAL(availableInstances(QList<shared_ptr<StreamInstance> >)), m_notifier, SLOT(notifyListeners(QList<shared_ptr<StreamInstance> >)));
    m_thread->start();
}

void PlaybackWorker::run()
{
    QElapsedTimer time;
    qint64 lastTime = 0;

    initialise();
    m_fps = 0;
    m_running = true;
    time.start();

    while (m_running)
    {
        // Compute time since last update
        qint64 timeNow = time.elapsed();
        qint64 diffTime = timeNow - lastTime;

        if (diffTime >= m_sleepTime)
        {
            // Compute Frame Per Seconds
            m_fps = 1.0 / (diffTime / 1000.0f);
            lastTime = timeNow;

            // Do job
            QList<shared_ptr<StreamInstance>>  readInstances = m_parent->readAllInstances();

            if (readInstances.count() > 0)
                emit availableInstances(readInstances);
        }
        else {
            QThread::currentThread()->msleep(m_sleepTime - diffTime);
        }
    }

    emit finished();
}

void PlaybackWorker::stop()
{
    m_running = false;
    std::cerr << "PlaybackWorker::stop()" << std::endl;
}

float PlaybackWorker::getFPS() const
{
    return m_fps;
}

} // End namespace
