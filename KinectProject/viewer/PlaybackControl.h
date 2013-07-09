#ifndef PLAYBACKCONTROL_H
#define PLAYBACKCONTROL_H

#include "types/StreamInstance.h"
#include "PlaybackWorker.h"
#include <QList>
#include <QHash>
#include <QMutex>

namespace dai {

class PlaybackControl : public QObject
{
    Q_OBJECT

    friend class PlaybackWorker;

public:

    class PlaybackListener
    {
    public:
        virtual void onNewFrame(QList<DataFrame*> frames) = 0;
    };

    PlaybackControl();
    virtual ~PlaybackControl();
    void stop();
    void play(bool restartAll = false);
    void addInstance(StreamInstance* instance);
    void enablePlayLoop(bool value);
    float getFPS() const;
    int acquire(QObject *caller);
    void release(QObject *caller, int token);
    void addNewFrameListener(PlaybackListener* listener, StreamInstance* instance);
    void removeListener(PlaybackListener* listener, StreamInstance::StreamType type);
    void removeListener(PlaybackListener* listener, StreamInstance* instance);
    void removeListener(PlaybackListener* listener);
    void setClearInstances(bool value);

signals:
    void onPlaybackFinished(PlaybackControl* playback);

private slots:
    void doWork();
    void stopAsync();

private:
    void removeAllListeners();
    void notifySuscribers(QList<StreamInstance*> notChangedInstances);

    PlaybackWorker*                                   m_worker;
    QMutex                                            m_lockViewers;
    int                                               m_viewers;
    QList<StreamInstance*>                            m_instances;
    bool                                              m_playloop_enabled;
    QHash<QObject*, int>                              m_usedTokens;
    QHash<PlaybackListener*, QList<StreamInstance*>*> m_listeners;
    QHash<StreamInstance*, QList<PlaybackListener*>*> m_listenersAux;
    QMutex                                            m_lockListeners;
    bool                                              m_clearInstances;
    bool                                              m_restartAfterStop;
};


} // End namespace

#endif // PLAYBACKCONTROL_H
