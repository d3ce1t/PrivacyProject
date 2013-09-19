#ifndef PLAYBACKWORKER_H
#define PLAYBACKWORKER_H

#include <QObject>
#include "PlaybackNotifier.h"
#include <memory>
#include "types/StreamInstance.h"
#include <QWaitCondition>
#include <QMutex>

using namespace std;

namespace dai {

class PlaybackControl;

class PlaybackWorker : public QObject
{
    Q_OBJECT

    friend class PlaybackNotifier;

public:
    PlaybackWorker(PlaybackControl *parent);
    void setFPS(float fps);
    float getFPS() const;
    void stop();

public slots:
    void run();

signals:
    void availableInstances(QList<shared_ptr<BaseInstance>> instances);
    void finished();

protected:
    void initialise();
    void sync();
    void waitForNotifier();
    void swap(const QList<shared_ptr<BaseInstance> > &instances);

private:
    bool                            m_running;
    qint64                          m_sleepTime;
    float                           m_fps;
    PlaybackControl*                m_parent;
    PlaybackNotifier*               m_notifier;
    QThread*                        m_thread;
    QMutex                          m_lockSync;
    QWaitCondition                  m_sync;
    bool                            m_notifierFinish;
};

} // End namespace

#endif // PLAYBACKWORKER_H
