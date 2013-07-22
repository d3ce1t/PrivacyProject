#ifndef PLAYBACKWORKER_H
#define PLAYBACKWORKER_H

#include <QThread>
#include <QElapsedTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QHash>
#include <atomic>

using namespace std;

namespace dai {

class PlaybackControl;
class PlaybackListener;

class PlaybackWorker : public QThread
{
    Q_OBJECT

    friend class PlaybackControl;
    friend class PlaybackListener;

public:
    PlaybackWorker(PlaybackControl *parent);
    virtual ~PlaybackWorker();
    float getFPS() const {return m_fps;}
    void run() Q_DECL_OVERRIDE;
    void sync();
    void stop();

private:
    void acquire(PlaybackListener* caller);
    void release(PlaybackListener* caller);


    bool             m_running;
    QMutex           m_lockViewers;
    atomic<int>      m_viewers;
    QHash<PlaybackListener*, bool> m_currentCallers;
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
