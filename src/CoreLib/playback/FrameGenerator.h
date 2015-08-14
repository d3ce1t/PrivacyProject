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

    // Listeners
    QReadWriteLock        m_listenersLock;
    QHash<FrameListener*, FrameNotifier*> m_listeners;

    // Stats
    QReadWriteLock        m_counterLock;
    qint64                m_frameCounter;
    QElapsedTimer         m_timer;
    float                 m_instantProductionRate; // 1 / Spent Time
    float                 m_productionRate; // 1 / Time between two calls

    // Buffers
    shared_ptr<QHashDataFrames> m_readBuffer;
    shared_ptr<QHashDataFrames> m_writeBuffer;
    bool                  m_doubleBuffer;
    bool                  m_initialised;

public:
    FrameGenerator();
    virtual ~FrameGenerator();
    void addListener(FrameListener* listener);
    void removeListener(FrameListener* listener);
    void begin(bool doubleBuffer = false);
    bool generate();

    // Frames que pueden ser generados por segundo
    inline float getGeneratorCapacity() const {return m_instantProductionRate;}

    // Frames generados por segundo
    inline float getFrameRate() const {return m_productionRate;}

    QElapsedTimer superTimer;

protected:
    void restartStats();
    qint64 productsCount();
    int subscribersCount() const;
    virtual shared_ptr<QHashDataFrames> allocateMemory() = 0;
    virtual void produceFrames(QHashDataFrames& output) = 0;

private:
    inline void notifyListeners(const QHashDataFrames &dataFrames, qint64 frameId);
    inline void swapBuffers();

    inline bool isValidFrame(qint64 frameIndex) {
        bool result = false;
        m_counterLock.lockForRead();
        if (frameIndex == m_frameCounter)
            result = true;
        m_counterLock.unlock();
        return result;
    }
};

} // End Namespace

#endif // NODEPRODUCER_H
