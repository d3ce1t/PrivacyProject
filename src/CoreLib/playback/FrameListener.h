#ifndef NODE_LISTENER_H
#define NODE_LISTENER_H

#include "types/DataFrame.h"
#include "FrameGenerator.h"
#include <QDebug>

namespace dai {

class FrameNotifier;

class FrameListener
{
    friend class FrameGenerator;
    friend class FrameNotifier;

    FrameGenerator* m_worker;  // PlaybackWorker::addListener sets this attribute
    qint64 m_lastFrameId;

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
    inline void newFrames(const QHashDataFrames dataFrames, const qint64 frameId) {
        // Check the received frames are valid because we could have been called out of time
        // frameId is the frame counter of the frame generator.
        if (!m_worker->isValidFrame(frameId)) {
            qDebug() << "FrameListener - Frame" << frameId << "received but discarded";
            return;
        }

        m_lastFrameId = frameId;
        newFrames(dataFrames);
    }
};

} // End Namespace

#endif // NODE_LISTENER_H
