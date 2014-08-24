#ifndef OPENNIUSERTRACKERINSTANCE_H
#define OPENNIUSERTRACKERINSTANCE_H

#include "OpenNIDevice.h"
#include "types/StreamInstance.h"
#include "types/SkeletonFrame.h"
#include "types/MaskFrame.h"
#include "types/DepthFrame.h"
#include "types/MetadataFrame.h"

namespace dai {

class OpenNIUserTrackerInstance : public StreamInstance
{
public:
    OpenNIUserTrackerInstance();
    OpenNIUserTrackerInstance(OpenNIDevice* device);
    virtual ~OpenNIUserTrackerInstance();
    bool is_open() const override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(QHashDataFrames& output) override;

private:
    OpenNIDevice* m_device;
};

} // End Namespace

#endif // OPENNIUSERTRACKERINSTANCE_H
