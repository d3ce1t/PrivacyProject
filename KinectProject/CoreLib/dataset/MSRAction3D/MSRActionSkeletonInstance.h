#ifndef MSR_ACTION_SKELETON_INSTANCE_H
#define MSR_ACTION_SKELETON_INSTANCE_H

#include "dataset/DataInstance.h"
#include "types/SkeletonFrame.h"
#include <fstream>

using namespace std;

namespace dai {

class MSRActionSkeletonInstance : public DataInstance<SkeletonFrame>
{
public:
    explicit MSRActionSkeletonInstance(const InstanceInfo& info);
    virtual ~MSRActionSkeletonInstance();
    bool is_open() const override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    QList<shared_ptr<DataFrame>> nextFrames() override;

private:
    static SkeletonJoint::JointType staticMap[20];

    ifstream         m_file;
    int              m_nJoints;
    shared_ptr<SkeletonFrame> m_frameBuffer;
};

} // End of namespace

#endif // MSR_ACTION_SKELETON_INSTANCE_H
