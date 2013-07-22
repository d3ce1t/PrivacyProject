#include "PlaybackListener.h"
#include "PlaybackControl.h"

namespace dai {

PlaybackListener::PlaybackListener()
{
    m_playback = nullptr;
}

PlaybackListener::~PlaybackListener()
{
    if (m_playback != nullptr) {
        m_playback->removeListener(this);
        m_playback = nullptr;
    }
}

void PlaybackListener::setPlayback(PlaybackControl* playback)
{
    m_playback = playback;
}

void PlaybackListener::manageFrames(QList<shared_ptr<DataFrame> > frames)
{
    onNewFrame(frames);
}

PlaybackControl* PlaybackListener::playback()
{
    return m_playback;
}

} // End Namespace
