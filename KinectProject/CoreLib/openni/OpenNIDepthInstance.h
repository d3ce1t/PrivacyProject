#ifndef OPENNIDEPTHINSTANCE_H
#define OPENNIDEPTHINSTANCE_H

#include "OpenNIDevice.h"
#include "types/StreamInstance.h"
#include "types/DepthFrame.h"
#include <QMutex>

namespace dai {

class OpenNIDepthInstance : public StreamInstance
{
public:
    OpenNIDepthInstance();
    OpenNIDepthInstance(OpenNIDevice* device);
    virtual ~OpenNIDepthInstance();
    bool is_open() const override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    QList<shared_ptr<DataFrame>> nextFrame() override;

private:
    OpenNIDevice* m_device;
};

} // End namespace

#endif // OPENNIDEPTHINSTANCE_H
