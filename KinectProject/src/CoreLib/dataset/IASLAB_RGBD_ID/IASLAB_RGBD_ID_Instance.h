#ifndef IASLAB_RGBD_ID_INSTANCE_H
#define IASLAB_RGBD_ID_INSTANCE_H

#include "dataset/DataInstance.h"
#include "types/SkeletonJoint.h"
#include "types/DepthFrame.h"
#include "types/MaskFrame.h"
#include "types/Skeleton.h"

namespace dai {

class IASLAB_RGBD_ID_Instance : public DataInstance
{
    static SkeletonJoint::JointType _staticMap[20];
    int  m_width;
    int  m_height;
    bool m_open;

public:
    explicit IASLAB_RGBD_ID_Instance(const InstanceInfo& info);
    virtual ~IASLAB_RGBD_ID_Instance();
    bool is_open() const override;
    bool hasNext() const override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(QHashDataFrames& output) override;

private:

    void depth2color(shared_ptr<DepthFrame> depthFrame, shared_ptr<MaskFrame> mask, shared_ptr<Skeleton> skeleton) const;

    // RGB Intrinsics
    const double fx_rgb = 525.0f;
    const double fy_rgb = -525.0f;
    const double cx_rgb = 310.0f;
    const double cy_rgb = 249.5f;

    // Depth Intrinsics
    const double fx_d = 575.8f;
    const double fy_d = -575.8f;
    const double cx_d = 314.5f;
    const double cy_d = 235.5f;
};

} // End namespace

#endif // IASLAB_RGBD_ID_INSTANCE_H
