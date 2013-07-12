#ifndef PLAYBACKCONTROL_H
#define PLAYBACKCONTROL_H

#include "types/StreamInstance.h"
#include <QObject>
#include <QList>
#include <QHash>
#include <QMutex>

namespace dai {

class PlaybackWorker;

class PlaybackControl : public QObject
{
    Q_OBJECT

    friend class PlaybackWorker;

public:

    class PlaybackListener
    {
        friend class PlaybackControl;

    public:
        PlaybackListener();
        virtual ~PlaybackListener();
        PlaybackControl* playback();
        void releasePlayback();
        virtual void onNewFrame(QList<DataFrame*> frames) = 0;
        virtual void onPlaybackStart() = 0;
        virtual void onPlaybackStop() = 0;
    private:
        void setPlayback(PlaybackControl* playback);

        PlaybackControl* m_playback;
    };

    PlaybackControl();
    virtual ~PlaybackControl();
    void stop();
    void play(bool restartAll = false);
    void addInstance(StreamInstance* instance);
    void enablePlayLoop(bool value);
    float getFPS() const;
    void addNewFrameListener(PlaybackListener* listener, StreamInstance* instance);
    void removeListener(PlaybackListener* listener, StreamInstance::StreamType type);
    void removeListener(PlaybackListener* listener, StreamInstance* instance);
    void removeListener(PlaybackListener* listener);
    void setClearInstances(bool value);

signals:
    void onPlaybackFinished(PlaybackControl* playback);

private slots:
    bool doWork();
    void stopAsync();

private:
    void release(PlaybackListener *caller);
    void removeAllListeners();
    void notifySuscribersOnNewFrames(QList<StreamInstance*> notChangedInstances);
    void notifySuscribersOnStop();

    PlaybackWorker*                                   m_worker;
    QList<StreamInstance*>                            m_instances;
    bool                                              m_playloop_enabled;
    QHash<PlaybackListener*, QList<StreamInstance*>*> m_listeners;
    QHash<StreamInstance*, QList<PlaybackListener*>*> m_listenersAux;
    QMutex                                            m_lockListeners;
    bool                                              m_clearInstances;
    bool                                              m_restartAfterStop;
};


} // End namespace

#endif // PLAYBACKCONTROL_H
