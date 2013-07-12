#ifndef PLAYBACKWORKER_H
#define PLAYBACKWORKER_H

#include <QThread>
#include <QElapsedTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QAtomicInt>
#include "PlaybackControl.h"

namespace dai {


class PlaybackWorker : public QThread
{
    Q_OBJECT

    friend class PlaybackControl;

public:
    PlaybackWorker(PlaybackControl *parent);
    virtual ~PlaybackWorker();
    float getFPS() const {return m_fps;}
    void run() Q_DECL_OVERRIDE;
    void sync();
    void stop();

private:
    void acquire(PlaybackControl::PlaybackListener *caller);
    void release(PlaybackControl::PlaybackListener *caller);


    bool             m_running;
    QMutex           m_lockViewers;
    QAtomicInt       m_viewers;
    QHash<PlaybackControl::PlaybackListener*, bool> m_currentCallers;
    QMutex           m_mutex;
    QWaitCondition   m_sync;
    qint64           SLEEP_TIME;
    QElapsedTimer    m_time;
    qint64           m_lastTime;
    long long        m_frames;
    float            m_fps;
    PlaybackControl* m_parent;
};

} // End namespace

#endif // PLAYBACKWORKER_H
