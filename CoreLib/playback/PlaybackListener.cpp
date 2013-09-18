#include "PlaybackListener.h"
#include "PlaybackControl.h"

namespace dai {

PlaybackListener::PlaybackListener()
{
    m_playback = nullptr;
    m_taskFinish = false;
}

PlaybackListener::~PlaybackListener()
{
    if (m_playback != nullptr) {
        m_playback->removeListener(this);
        PlaybackListener::endAsyncTask();
        m_playback = nullptr;
    }
}

void PlaybackListener::startAsyncTask()
{
    m_lockSync.lock();
    while (!m_taskFinish) {
        m_sync.wait(&m_lockSync);
    }
    m_taskFinish = false;
    m_lockSync.unlock();
}

void PlaybackListener::endAsyncTask()
{
    m_lockSync.lock();
    if (!m_taskFinish) {
        m_taskFinish = true;
        m_sync.wakeOne();
    }
    m_lockSync.unlock();
}

void PlaybackListener::setPlayback(PlaybackControl* playback)
{
    m_playback = playback;
}

PlaybackControl* PlaybackListener::playback()
{
    return m_playback;
}

} // End Namespace
