#ifndef MSRDAILYSKELETONINSTANCE_H
#define MSRDAILYSKELETONINSTANCE_H

#include "types/DataInstance.h"
#include "types/SkeletonFrame.h"
#include <fstream>

using namespace std;

namespace dai {

class MSRDailySkeletonInstance : public DataInstance<SkeletonFrame>
{
public:
    explicit MSRDailySkeletonInstance(const InstanceInfo& info);
    virtual ~MSRDailySkeletonInstance();
    bool is_open() const override;

protected:
    void openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(SkeletonFrame& frame) override;

private:
    static SkeletonJoint::JointType staticMap[20];

    ifstream         m_file;
    int              m_nJoints;
    shared_ptr<SkeletonFrame> m_frameBuffer[2];
};

} // End of namespace

#endif // MSRDAILYSKELETONINSTANCE_H
