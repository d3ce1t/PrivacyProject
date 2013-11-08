#ifndef PLAYBACKLISTENER_H
#define PLAYBACKLISTENER_H

#include <memory>
#include <types/DataFrame.h>
#include <QHash>
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
    // This method must be synchronous, so notifier block until it ends
    virtual void onNewFrame(const QHash<DataFrame::FrameType, shared_ptr<DataFrame>>& frames) = 0;
    virtual void onPlaybackStart() {}
    virtual void onPlaybackStop() {}

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
