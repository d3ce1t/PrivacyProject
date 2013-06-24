#ifndef PLAYBACKWORKER_H
#define PLAYBACKWORKER_H

#include <QThread>
#include <QElapsedTimer>
#include <QMutex>
#include <QWaitCondition>

namespace dai {

class PlaybackControl;

class PlaybackWorker : public QThread
{
    Q_OBJECT
public:
    PlaybackWorker(PlaybackControl *parent);
    float getFPS() const {return m_fps;}
    void run() Q_DECL_OVERRIDE;
    void sync();
    void stop();

private:
    bool             m_running;
    QMutex           m_mutex;
    QWaitCondition   m_sync;
    const qint64     SLEEP_TIME;
    QElapsedTimer    m_time;
    qint64           m_lastTime;
    long long        m_frames;
    float            m_fps;
    PlaybackControl* m_parent;
};

} // End namespace

#endif // PLAYBACKWORKER_H
