#ifndef NODE_LISTENER_H
#define NODE_LISTENER_H

#include "types/DataFrame.h"
#include "NodeProducer.h"
#include <QFuture>

namespace dai {

class NodeListener
{
    friend class NodeProducer;

public:
    virtual ~NodeListener();

    /**
     * This method is called from the ListenerNotifier thread assigned to each PlaybackListener
     *
     * @brief newFrames
     * @param dataFrames
     * @param frameId
     * @param availableTime
     */
    virtual void newFrames(const QHashDataFrames dataFrames) = 0;

protected:
    NodeProducer* producerHandler();
    bool hasExpired();
    void stopListener();

private:
    void newFrames(const QHashDataFrames dataFrames, const qint64 frameId);
    NodeProducer* m_worker;  // PlaybackWorker::addListener sets this attribute
    QFuture<void> m_future; // PlaybackWorker::notifyListeners sets this attribute
    qint64 m_lastFrameId;
};

} // End Namespace

#endif // NODE_LISTENER_H
