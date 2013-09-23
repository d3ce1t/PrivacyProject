#ifndef PLAYBACKCONTROL_H
#define PLAYBACKCONTROL_H

#include "types/StreamInstance.h"
#include "PlaybackListener.h"
#include "PlaybackWorker.h"
#include "PlaybackNotifier.h"
#include <QThread>
#include <QObject>
#include <QList>
#include <QMultiHash>
#include <QMutex>
#include <memory>

using namespace std;

namespace dai {

class PlaybackControl : public QObject
{
    Q_OBJECT

    friend class PlaybackWorker;
    friend class PlaybackListener;
    friend class PlaybackNotifier;

public:
    PlaybackControl();
    virtual ~PlaybackControl();

    // Playback Control
    void setFPS(float fps);
    void play(bool restartAll = false);
    void stop();
    void enablePlayLoop(bool value);
    void addInstance(shared_ptr<BaseInstance> instance);
    void removeInstance(shared_ptr<BaseInstance> instance);

    // Listeners
    void addListener(PlaybackListener* listener, shared_ptr<BaseInstance> instance);
    void removeListener(PlaybackListener* listener, InstanceType type);
    void removeListener(PlaybackListener* listener, shared_ptr<BaseInstance> instance);
    void removeListener(PlaybackListener* listener);

    // Query
    float getFPS() const;

signals:
    void onPlaybackFinished(PlaybackControl* playback);

private slots:
    void stopAsync();

private:
    QList<shared_ptr<BaseInstance> > readAllInstances();
    void notifyListeners(QList<shared_ptr<BaseInstance> > changedInstances);
    void notifySuscribersOnStop();
    void removeAllListeners();

    // Worker
    PlaybackWorker*                                    m_worker;
    QThread*                                           m_thread;

    // Instances and Listeners
    QList<shared_ptr<BaseInstance>>                    m_instances;
    QList<PlaybackListener*>                           m_listeners;
    QMultiHash<PlaybackListener*, shared_ptr<BaseInstance>> m_listenerToInstanceMap;
    QMultiHash<BaseInstance*, PlaybackListener*>       m_instanceToListenerMap;
    QMutex                                             m_lockListeners;

    // Playback Setting Options
    bool                                               m_playloop_enabled;
    bool                                               m_restartAfterStop;
    float                                              m_fps;
};

} // End namespace

#endif // PLAYBACKCONTROL_H
