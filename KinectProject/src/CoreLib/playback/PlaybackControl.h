#ifndef PLAYBACKCONTROL_H
#define PLAYBACKCONTROL_H

#include <QThread>
#include "types/StreamInstance.h"
#include <memory>

using namespace std;

namespace dai {

class FrameListener;
class PlaybackWorker;

class PlaybackControl
{
public:
    PlaybackControl();
    virtual ~PlaybackControl();
    void addListener(FrameListener* listener);
    void removeListener(FrameListener* listener);
    bool addInstance(shared_ptr<StreamInstance> instance);
    void removeInstance(shared_ptr<StreamInstance> instance);
    void clearInstances();
    void enablePlayLoop(bool value);
    void setFPS(float fps);

// These could be slots
    void play(bool restartAll = false);
    void stop();
    void pause();

private:
    QThread         m_workerThread;
    PlaybackWorker *m_worker;
};

} // End namespace

#endif // PLAYBACKCONTROL_H
