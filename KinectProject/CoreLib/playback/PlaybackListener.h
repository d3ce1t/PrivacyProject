#ifndef PLAYBACKLISTENER_H
#define PLAYBACKLISTENER_H

#include "types/DataFrame.h"
#include "PlaybackWorker.h"
#include <QFuture>

namespace dai {

class PlaybackListener
{
    friend class PlaybackWorker;

public:
    virtual ~PlaybackListener();

    /**
     * This method is called from the ListenerNotifier thread assigned to each PlaybackListener
     *
     * @brief newFrames
     * @param dataFrames
     * @param frameId
     * @param availableTime
     */
    virtual void newFrames(const QHashDataFrames dataFrames, const qint64 frameId) = 0;

protected:
    PlaybackWorker* playbackHandler();
    void stopListener();

private:
    PlaybackWorker* m_worker;  // PlaybackWorker::addListener sets this attribute
    QFuture<void> m_future; // PlaybackWorker::notifyListeners sets this attribute
};

} // End Namespace

#endif // PLAYBACKLISTENER_H
