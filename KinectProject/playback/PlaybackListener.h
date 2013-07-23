#ifndef PLAYBACKLISTENER_H
#define PLAYBACKLISTENER_H

#include <memory>
#include <types/DataFrame.h>
#include <QWaitCondition>
#include <QMutex>

using namespace std;

namespace dai {

class PlaybackControl;

class PlaybackListener
{
    friend class PlaybackControl;

public:
    PlaybackListener();
    virtual ~PlaybackListener();
    PlaybackControl* playback();

protected:
    virtual void onNewFrame(const QList<shared_ptr<DataFrame>>& frames) = 0;
    virtual void onPlaybackStart() = 0;
    virtual void onPlaybackStop() = 0;

    void startAsyncTask();
    void endAsyncTask();

private:
    void setPlayback(PlaybackControl* playback);

    PlaybackControl* m_playback;
    QMutex           m_lockSync;
    QWaitCondition   m_sync;
    bool             m_taskFinish;
};

} // End Namespace

#endif // PLAYBACKLISTENER_H
