#ifndef PLAYBACKWORKER_H
#define PLAYBACKWORKER_H

#include <QObject>
#include <QList>
#include <QReadWriteLock>
#include <memory>
#include "types/StreamInstance.h"
#include <QElapsedTimer>

using namespace std;

namespace dai {

class PlaybackListener;

class PlaybackWorker : public QObject
{
    Q_OBJECT

    friend class PlaybackControl;
    friend class PlaybackListener;

public:
    PlaybackWorker();
    ~PlaybackWorker();
    float getFPS() const;
    bool isValidFrame(qint64 frameIndex);
    QElapsedTimer superTimer;

public slots:
    void run();
    void stop();

private:
    void enablePlayLoop(bool value);
    void setFPS(float fps);
    void addInstance(shared_ptr<StreamInstance> instance);
    void removeInstance(shared_ptr<StreamInstance> instance);
    void clearInstances();
    void addListener(PlaybackListener* listener);
    void removeListener(PlaybackListener* listener);
    void setupListeners();
    bool notifyListeners(QList<shared_ptr<StreamInstance>> instances);
    void openAllInstances();
    void closeAllInstances();
    QList<shared_ptr<StreamInstance>> readAllInstances();

    QList<shared_ptr<StreamInstance>>  m_instances;
    QList<PlaybackListener*>           m_listeners;
    bool                               m_playloop_enabled;
    qint64                             m_slotTime;
    bool                               m_running;
    float                              m_fps;
    QReadWriteLock                     m_counterLock;
    QReadWriteLock                     m_listenersLock;
    qint64                             m_framesCounter;
};

} // End Namespace

#endif // PLAYBACKWORKER_H
