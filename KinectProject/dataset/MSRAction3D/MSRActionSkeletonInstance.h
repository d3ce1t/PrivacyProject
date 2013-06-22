#ifndef MSR_ACTION_SKELETON_INSTANCE_H
#define MSR_ACTION_SKELETON_INSTANCE_H

#include "dataset/DataInstance.h"
#include "types/Skeleton.h"
#include <fstream>

using namespace std;

namespace dai {

class MSRActionSkeletonInstance : public DataInstance
{
public:
    explicit MSRActionSkeletonInstance(const InstanceInfo& info);
    virtual ~MSRActionSkeletonInstance();
    bool is_open() const Q_DECL_OVERRIDE;
    void open() Q_DECL_OVERRIDE;
    void close() Q_DECL_OVERRIDE;
    dai::Skeleton& frame() Q_DECL_OVERRIDE;

protected:
    void nextFrame(DataFrame& frame) Q_DECL_OVERRIDE;
    void restart() Q_DECL_OVERRIDE;

private:
    static SkeletonJoint::JointType convertIntToType(int value);

    ifstream         m_file;
    int              m_nJoints;
    dai::Skeleton    m_frameBuffer[2];
};

} // End of namespace

#endif // MSR_ACTION_SKELETON_INSTANCE_H
