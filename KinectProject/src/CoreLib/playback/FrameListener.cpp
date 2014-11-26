#include "FrameListener.h"
#include "FrameNotifier.h"

namespace dai {

FrameListener::FrameListener()
    : m_worker(nullptr)
    , m_lastFrameId(0)
{
}

FrameListener::~FrameListener()
{
    qDebug() << "FrameListener::~FrameListener";
    stopListener();
    m_worker = nullptr;
}

bool FrameListener::hasExpired()
{
    bool result = true;

    if (m_worker)
        result = !m_worker->isValidFrame(m_lastFrameId);

    return result;
}

FrameGenerator *FrameListener::producerHandler()
{
    return m_worker;
}

void FrameListener::stopListener()
{
    if (m_worker)
        m_worker->removeListener(this); // Blocks until last job ends
}

} // End Namespace
