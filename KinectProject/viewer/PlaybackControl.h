#ifndef PLAYBACKCONTROL_H
#define PLAYBACKCONTROL_H

#include "types/StreamInstance.h"
#include "PlaybackListener.h"
#include "PlaybackWorker.h"
#include <QObject>
#include <QList>
#include <QHash>
#include <QMutex>
#include <memory>

using namespace std;

namespace dai {

class PlaybackControl : public QObject
{
    Q_OBJECT

    friend class PlaybackWorker;
    friend class PlaybackListener;

public:
    PlaybackControl();
    virtual ~PlaybackControl();
    void stop();
    void play(bool restartAll = false);
    void addInstance(shared_ptr<StreamInstance> instance);
    void enablePlayLoop(bool value);
    float getFPS() const;
    void addListener(PlaybackListener* listener, shared_ptr<StreamInstance> instance);
    void removeListener(PlaybackListener* listener, StreamInstance::StreamType type);
    void removeListener(PlaybackListener* listener, shared_ptr<StreamInstance> instance);
    void removeListener(PlaybackListener* listener);

signals:
    void onPlaybackFinished(PlaybackControl* playback);

private slots:
    bool doWork();
    void stopAsync();

private:
    bool hasSuscribers(shared_ptr<StreamInstance> instance);
    void removeAllListeners();
    void notifySuscribersOnNewFrames(QList<shared_ptr<StreamInstance> > notChangedInstances);
    void notifySuscribersOnStop();

    PlaybackWorker*                                   m_worker;
    QList<shared_ptr<StreamInstance> >                m_instances;
    bool                                              m_playloop_enabled;
    QHash<PlaybackListener*, QList<shared_ptr<StreamInstance> >*> m_listeners;
    QMutex                                            m_lockListeners;
    bool                                              m_restartAfterStop;
};


} // End namespace

#endif // PLAYBACKCONTROL_H
