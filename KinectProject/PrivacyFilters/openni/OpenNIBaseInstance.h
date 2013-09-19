#ifndef OPENNIBASEINSTANCE_H
#define OPENNIBASEINSTANCE_H

#include <QMutex>
#include <QWaitCondition>

namespace dai {

class OpenNIBaseInstance
{
public:
    OpenNIBaseInstance();

protected:
    void computeStats(unsigned int frameIndex);
    void notifyNewFrame();
    void waitForNewFrame();

private:
    unsigned int    m_lastFrameId;
    QMutex          m_lockSync;
    QWaitCondition  m_sync;
    bool            m_newFrameGenerated;
};

} // End namespace

#endif // OPENNIBASEINSTANCE_H
