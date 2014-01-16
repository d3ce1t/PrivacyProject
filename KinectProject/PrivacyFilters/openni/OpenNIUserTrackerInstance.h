#ifndef OPENNIUSERTRACKERINSTANCE_H
#define OPENNIUSERTRACKERINSTANCE_H

#include "OpenNIRuntime.h"
#include "UserTrackerFrame.h"
#include "types/BaseInstance.h"
#include <QReadWriteLock>

namespace dai {

class OpenNIUserTrackerInstance : public BaseInstance
{
public:
    OpenNIUserTrackerInstance();
    virtual ~OpenNIUserTrackerInstance();
    void open() override;
    void close() override;
    void restart() override;
    bool is_open() const override;
    bool hasNext() const override;
    void swapBuffer() override;
    void readNextFrame() override;
    QList< shared_ptr<DataFrame> > frames() override;

private:
    void nextFrame(UserTrackerFrame& frame);

private:
    OpenNIRuntime*    m_openni;
    UserTrackerFrame* m_frameBuffer[2];
    QReadWriteLock    m_locker;
    UserTrackerFrame* m_readFrame;
    UserTrackerFrame* m_writeFrame;
};

} // End Namespace

#endif // OPENNIUSERTRACKERINSTANCE_H
