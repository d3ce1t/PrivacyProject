#ifndef FRAMENOTIFIER_H
#define FRAMENOTIFIER_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include "types/DataFrame.h"

namespace dai {

class FrameListener;

class FrameNotifier : public QThread
{
public:
    FrameNotifier(FrameListener* listener);
    ~FrameNotifier();
    void notifyListener(const QHashDataFrames data, const qint64 frameId);
    void stop();

protected:
    void run() override;

private:
    bool waitingForNewOrder();
    void done();

    FrameListener* m_listener;
    QHashDataFrames m_data;
    qint64 m_frameId;
    bool m_running;
    QWaitCondition m_sync;
    QMutex m_syncLock;
    QMutex m_loopLock;
    bool m_workInProgress;

};

} // End Namespace

#endif // FRAMENOTIFIER_H
