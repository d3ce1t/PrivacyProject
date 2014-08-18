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
    QList<shared_ptr<DataFrame>> nextFrame() override;

private:
    OpenNIDevice* m_device;
    shared_ptr<SkeletonFrame> m_frameSkeleton;
    shared_ptr<MetadataFrame> m_frameMetadata;
};

} // End Namespace

#endif // OPENNIUSERTRACKERINSTANCE_H
