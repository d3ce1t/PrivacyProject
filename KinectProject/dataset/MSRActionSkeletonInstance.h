#ifndef MSR_ACTION_SKELETON_INSTANCE_H
#define MSR_ACTION_SKELETON_INSTANCE_H

#include "DataInstance.h"
#include "../types/Skeleton.h"
#include <fstream>

using namespace std;

namespace dai {

class MSRActionSkeletonInstance : public DataInstance
{
public:
    explicit MSRActionSkeletonInstance(const InstanceInfo& info);
    virtual ~MSRActionSkeletonInstance();
    void open();
    void close();
    int getTotalFrames() const;
    bool hasNext() const;
    const dai::Skeleton &nextFrame();

private:
    static SkeletonJoint::JointType convertIntToType(int value);

    ifstream    m_file;
    int         m_nFrames;
    int         m_frameIndex;
    int         m_nJoints;
    dai::Skeleton    m_currentFrame;
};

} // End of namespace

#endif // MSR_ACTION_SKELETON_INSTANCE_H
