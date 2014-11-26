#ifndef PLAYBACKWORKER_H
#define PLAYBACKWORKER_H

#include <QObject>
#include "FrameGenerator.h"
#include <QList>
#include <memory>
#include "types/StreamInstance.h"

using namespace std;

namespace dai {

class FrameListener;

class PlaybackWorker : public QObject, public FrameGenerator
{
    Q_OBJECT

    friend class PlaybackControl;
    friend class FrameListener;

public:
    PlaybackWorker();
    ~PlaybackWorker();

public slots:
    void run();
    void stop();

protected:
    void produceFrames(QHashDataFrames &output) override;
    shared_ptr<QHashDataFrames> allocateMemory() override;

private:
    void enablePlayLoop(bool value);
    void setFPS(float fps);
    bool addInstance(shared_ptr<StreamInstance> instance);
    void removeInstance(shared_ptr<StreamInstance> instance);
    void clearInstances();

    void openAllInstances();
    void closeAllInstances();

    QList<shared_ptr<StreamInstance>>  m_instances;
    bool                               m_playloop_enabled;
    qint64                             m_slotTime;
    bool                               m_running;
    DataFrame::SupportedFrames         m_supportedFrames;
};

} // End Namespace

#endif // PLAYBACKWORKER_H
