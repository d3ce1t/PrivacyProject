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
    PlaybackControl();
    virtual ~PlaybackControl();
    void stop();
    void play(bool restartAll = false);
    void addInstance(StreamInstance* instance);
    void enablePlayLoop(bool value);
    float getFPS() const;
    int acquire();
    void release(int token);

signals:
    void newFrameRead();

private:
    void doWork();

    PlaybackWorker          m_worker;
    QMutex                  m_mutex;
    int                     m_viewers;
    QList<StreamInstance*>  m_instances;
    bool                    m_playloop_enabled;
    int                     m_token;
    bool                    m_usedToken[200];
};



} // End namespace

#endif // PLAYBACKCONTROL_H
