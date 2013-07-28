#ifndef MSR_ACTION_SKELETON_INSTANCE_H
#define MSR_ACTION_SKELETON_INSTANCE_H

#include "dataset/DataInstance.h"
#include "types/SkeletonFrame.h"
#include <fstream>

using namespace std;

namespace dai {

class MSRActionSkeletonInstance : public DataInstance
{
public:
    explicit MSRActionSkeletonInstance(const InstanceInfo& info);
    virtual ~MSRActionSkeletonInstance();
    bool is_open() const override;

protected:
    void openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(DataFrame& frame) override;

private:
    static SkeletonJoint::JointType staticMap[20];

    ifstream         m_file;
    int              m_nJoints;
    shared_ptr<SkeletonFrame> m_frameBuffer[2];
};

} // End of namespace

#endif // MSR_ACTION_SKELETON_INSTANCE_H
