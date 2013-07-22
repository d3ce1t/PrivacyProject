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

    // Playback Control
    void play(bool restartAll = false);
    void stop();
    void enablePlayLoop(bool value);
    void addInstance(shared_ptr<StreamInstance> instance);

    // Listeners
    void addListener(PlaybackListener* listener, shared_ptr<StreamInstance> instance);
    void removeListener(PlaybackListener* listener, StreamInstance::StreamType type);
    void removeListener(PlaybackListener* listener, shared_ptr<StreamInstance> instance);
    void removeListener(PlaybackListener* listener);

    // Query
    float getFPS() const;

signals:
    void onPlaybackFinished(PlaybackControl* playback);

private slots:
    void stopAsync();

private:
    bool readAllInstances();
    bool hasSuscribers(shared_ptr<StreamInstance> instance);
    void notifySuscribersOnNewFrames(QList<shared_ptr<StreamInstance> > notChangedInstances);
    void notifySuscribersOnStop();
    void removeAllListeners();

    PlaybackWorker*                                   m_worker;
    QList<shared_ptr<StreamInstance> >                m_instances;
    QHash<PlaybackListener*, QList<shared_ptr<StreamInstance> >*> m_listeners;
    QMutex                                            m_lockListeners;

    // Playback Setting Options
    bool                                              m_playloop_enabled;
    bool                                              m_restartAfterStop;
};


} // End namespace

#endif // PLAYBACKCONTROL_H
