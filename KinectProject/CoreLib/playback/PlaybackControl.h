#ifndef PLAYBACKCONTROL_H
#define PLAYBACKCONTROL_H

#include <QObject>
#include <QList>
#include "types/BaseInstance.h"
#include "playback/PlaybackWorker.h"
#include <QThread>
#include <memory>

using namespace std;

namespace dai {

class PlaybackControl : public QObject
{
    Q_OBJECT

public:
    PlaybackControl();
    virtual ~PlaybackControl();
    void addInstance(shared_ptr<BaseInstance> instance);
    void removeInstance(shared_ptr<BaseInstance> instance);
    void enablePlayLoop(bool value);
    void setFPS(float fps);
    float getFPS() const;
    bool isValidFrame(qint64 frameIndex) const;

public slots:
    void play(bool restartAll = false);
    void stop();

signals:
    void onNewFrames(const QMultiHashDataFrames dataFrames, const qint64 frameId, const PlaybackControl* playback);
    void onStop();

private:
    QThread                         m_workerThread;
    PlaybackWorker                  *m_worker;
    QList<shared_ptr<BaseInstance>> m_instances;
};

} // End namespace

#endif // PLAYBACKCONTROL_H
