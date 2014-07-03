#include "PlaybackListener.h"
#include <QDebug>

namespace dai {

PlaybackListener::~PlaybackListener()
{
    qDebug() << "PlaybackListener::~PlaybackListener";
    stopListener();
}

PlaybackWorker* PlaybackListener::playbackHandler()
{
    return m_worker;
}

void PlaybackListener::stopListener()
{
    m_worker->removeListener(this);
    m_future.waitForFinished();
}

} // End Namespace
