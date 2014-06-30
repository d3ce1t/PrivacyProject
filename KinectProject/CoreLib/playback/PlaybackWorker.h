#ifndef PLAYBACKWORKER_H
#define PLAYBACKWORKER_H

#include <QObject>
#include <QList>
#include <QReadWriteLock>
#include <memory>
#include "types/BaseInstance.h"

using namespace std;

namespace dai {

class PlaybackControl;

class PlaybackWorker : public QObject
{
    Q_OBJECT

public:
    PlaybackWorker(const PlaybackControl* playback, QList<shared_ptr<BaseInstance>>& instances);
    void enablePlayLoop(bool value);
    void setFPS(float fps);
    float getFPS() const;
    bool isValidFrame(qint64 frameIndex);

public slots:
    void run();
    void stop();

signals:
    void onNewFrames(const QHashDataFrames dataFrames, const qint64 frameId, const PlaybackControl* playback);
    void onStop();

private:
    void openAllInstances();
    void closeAllInstances();
    QList<shared_ptr<BaseInstance> > readAllInstances();
    void swapAllInstances(const QList<shared_ptr<BaseInstance> > &instances);

    const PlaybackControl*           m_playback;
    QList<shared_ptr<BaseInstance>>& m_instances;
    bool                             m_playloop_enabled;
    qint64                           m_slotTime;
    bool                             m_running;
    float                            m_fps;
    QReadWriteLock                   m_lock;
    qint64                           m_framesCounter;
};

} // End Namespace

#endif // PLAYBACKWORKER_H
