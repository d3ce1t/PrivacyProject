#ifndef PLAYBACKWORKER_H
#define PLAYBACKWORKER_H

#include <QObject>

namespace dai {

class PlaybackControl;

class PlaybackWorker : public QObject
{
    Q_OBJECT

public:
    PlaybackWorker(PlaybackControl *parent);
    float getFPS() const;
    void stop();

public slots:
    void run();

signals:
    void finished();

private:
    bool                            m_running;
    qint64                          m_sleepTime;
    float                           m_fps;
    PlaybackControl*                m_parent;
};

} // End namespace

#endif // PLAYBACKWORKER_H
