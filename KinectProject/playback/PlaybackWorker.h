#ifndef PLAYBACKWORKER_H
#define PLAYBACKWORKER_H

#include <QThread>

namespace dai {

class PlaybackControl;

class PlaybackWorker : public QThread
{
    Q_OBJECT

public:
    PlaybackWorker(PlaybackControl *parent);
    float getFPS() const;
    void run() override;
    void stop();

private:
    bool                            m_running;
    qint64                          m_sleepTime;
    float                           m_fps;
    PlaybackControl*                m_parent;
};

} // End namespace

#endif // PLAYBACKWORKER_H
