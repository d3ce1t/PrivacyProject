#ifndef PLAYBACKCONTROL_H
#define PLAYBACKCONTROL_H

#include <QObject>
#include <QList>
#include "playback/PlaybackWorker.h"
#include <QThread>
#include <memory>
#include <QElapsedTimer>

using namespace std;

namespace dai {

class PlaybackControl : public QObject
{
    Q_OBJECT

public:
    PlaybackControl();
    virtual ~PlaybackControl();
    void addInstance(shared_ptr<StreamInstance> instance);
    void removeInstance(shared_ptr<StreamInstance> instance);
    void enablePlayLoop(bool value);
    void setFPS(float fps);
    float getFPS() const;
    bool isValidFrame(qint64 frameIndex) const;
    PlaybackWorker* worker() const;

    QElapsedTimer superTimer;

public slots:
    void play(bool restartAll = false);
    void stop();
    void clearInstances();

private:
    QThread                           m_workerThread;
    PlaybackWorker                   *m_worker;
    QList<shared_ptr<StreamInstance>> m_instances;
};

} // End namespace

#endif // PLAYBACKCONTROL_H
