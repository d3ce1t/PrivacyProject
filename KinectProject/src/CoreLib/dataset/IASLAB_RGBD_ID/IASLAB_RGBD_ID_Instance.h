#ifndef IASLAB_RGBD_ID_INSTANCE_H
#define IASLAB_RGBD_ID_INSTANCE_H

#include "dataset/DataInstance.h"
#include "types/SkeletonJoint.h"

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
};

} // End namespace

#endif // IASLAB_RGBD_ID_INSTANCE_H
