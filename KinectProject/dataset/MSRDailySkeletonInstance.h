#ifndef MSRDAILYSKELETONINSTANCE_H
#define MSRDAILYSKELETONINSTANCE_H

#include "DataInstance.h"
#include "types/Skeleton.h"
#include <fstream>

using namespace std;

namespace dai {

class MSRDailySkeletonInstance : public DataInstance
{
public:
    explicit MSRDailySkeletonInstance(const InstanceInfo& info);
    virtual ~MSRDailySkeletonInstance();
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

#endif // MSRDAILYSKELETONINSTANCE_H
