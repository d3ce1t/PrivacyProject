#ifndef NODEPRODUCER_H
#define NODEPRODUCER_H

#include "types/DataFrame.h"
#include <QList>
#include <QReadWriteLock>
#include <QElapsedTimer>

namespace dai {

class NodeListener;

class NodeProducer
{
    friend class NodeListener;

public:
    NodeProducer();
    virtual ~NodeProducer();
    void addListener(NodeListener* listener);
    void removeListener(NodeListener* listener);
    bool produce();
    float getProductionCapacity() const; // Productos que pueden ser generados por segundo
    float getProductionRate() const; // Productos generados por segundo
    QElapsedTimer superTimer;

protected:
    qint64 productsCount();
    int subscribersCount() const;
    virtual QHashDataFrames produceFrames() = 0;

private:
    void notifyListeners(const QHashDataFrames dataFrames);
    bool isValidProduct(qint64 frameIndex);

    QReadWriteLock       m_listenersLock;
    QList<NodeListener*> m_listeners;
    QReadWriteLock       m_counterLock;
    qint64               m_productsCounter;
    QElapsedTimer        m_timer;
    qint64               m_lastTime;
    float                m_instantProductionRate; // 1 / Spent Time
    float                m_productionRate; // 1 / Time between two calls
};

} // End Namespace

#endif // NODEPRODUCER_H
