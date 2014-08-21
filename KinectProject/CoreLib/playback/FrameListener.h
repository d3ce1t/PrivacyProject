#ifndef NODE_LISTENER_H
#define NODE_LISTENER_H

#include "types/DataFrame.h"
#include "FrameGenerator.h"

namespace dai {

class FrameNotifier;

class FrameListener
{
    friend class FrameGenerator;
    friend class FrameNotifier;

public:
    FrameListener();
    virtual ~FrameListener();

protected:
    /**
     * This method is called from the ListenerNotifier thread assigned to each PlaybackListener
     *
     * @brief newFrames
     * @param dataFrames
     * @param frameId
     * @param availableTime
     */
    virtual void newFrames(const QHashDataFrames dataFrames) = 0;

    bool hasExpired();
    virtual void afterStop() {}
    FrameGenerator* producerHandler();
    void stopListener();

private:
    void newFrames(const QHashDataFrames dataFrames, const qint64 frameId);
    FrameGenerator* m_worker;  // PlaybackWorker::addListener sets this attribute
    qint64 m_lastFrameId;
};

} // End Namespace

#endif // NODE_LISTENER_H
