#ifndef NODEPRODUCER_H
#define NODEPRODUCER_H

#include "types/DataFrame.h"
#include <QHash>
#include <QReadWriteLock>
#include <QElapsedTimer>

namespace dai {

class FrameListener;
class FrameNotifier;

class FrameGenerator
{
    friend class FrameListener;

public:
    FrameGenerator();
    virtual ~FrameGenerator();
    void addListener(FrameListener* listener);
    void removeListener(FrameListener* listener);
    bool generate();

    // Frames que pueden ser generados por segundo
    inline float getGeneratorCapacity() const {return m_instantProductionRate;}

    // Frames generados por segundo
    float getFrameRate() const {return m_productionRate;}

    QElapsedTimer superTimer;

protected:
    void restartStats();
    qint64 productsCount();
    int subscribersCount() const;
    virtual QHashDataFrames produceFrames() = 0;

private:
    inline void notifyListeners(const QHashDataFrames &dataFrames, qint64 frameId);

    inline bool isValidFrame(qint64 frameIndex)
    {
        bool result = false;
        m_counterLock.lockForRead();
        if (frameIndex == m_frameCounter)
            result = true;
        m_counterLock.unlock();
        return result;
    }

    QReadWriteLock        m_listenersLock;
    QHash<FrameListener*, FrameNotifier*> m_listeners;
    QReadWriteLock        m_counterLock;
    qint64                m_frameCounter;
    QElapsedTimer         m_timer;
    float                 m_instantProductionRate; // 1 / Spent Time
    float                 m_productionRate; // 1 / Time between two calls
};

} // End Namespace

#endif // NODEPRODUCER_H
