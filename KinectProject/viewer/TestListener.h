#ifndef TESTLISTENER_H
#define TESTLISTENER_H

#include "playback/PlaybackListener.h"

namespace dai {

class TestListener : public PlaybackListener
{
public:
    void onNewFrame(const QList<shared_ptr<DataFrame>>& frames);
    void onPlaybackStart();
    void onPlaybackStop();
};

} // End namespace

#endif // TESTLISTENER_H
