#ifndef OPENNIDEPTHINSTANCE_H
#define OPENNIDEPTHINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/DepthFrame.h"
#include "openni/OpenNIBaseInstance.h"
#include <QMutex>

namespace dai {

class OpenNIDepthInstance : public StreamInstance<DepthFrame>,
                            public OpenNIBaseInstance
{
public:
    OpenNIDepthInstance();
    virtual ~OpenNIDepthInstance();
    bool is_open() const override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(DepthFrame& frame) override;

private:
    OpenNIRuntime*          m_openni;
    shared_ptr<DepthFrame>  m_frameBuffer[2];
};

} // End namespace

#endif // OPENNIDEPTHINSTANCE_H
