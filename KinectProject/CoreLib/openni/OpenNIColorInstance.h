#ifndef OPENNICOLORINSTANCE_H
#define OPENNICOLORINSTANCE_H

#include "openni/OpenNIDevice.h"
#include "types/StreamInstance.h"
#include "types/ColorFrame.h"

namespace dai {

class OpenNIColorInstance : public StreamInstance
{
public:
    OpenNIColorInstance();
    OpenNIColorInstance(OpenNIDevice* device);
    virtual ~OpenNIColorInstance();
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

#endif // OPENNICOLORINSTANCE_H
