#ifndef INSTANCERECORDER_H
#define INSTANCERECORDER_H

#include "playback/PlaybackListener.h"
#include <QFile>

namespace dai {

class InstanceRecorder : public PlaybackListener
{
public:
    InstanceRecorder();
    void onNewFrame(QList<DataFrame*> dataFrames);
    void onPlaybackStart();
    void onPlaybackStop();

private:
    QFile m_of;
    unsigned int m_lastFrame;
};

} // End Namespace

#endif // INSTANCERECORDER_H
