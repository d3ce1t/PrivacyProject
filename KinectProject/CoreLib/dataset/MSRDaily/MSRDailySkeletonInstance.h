#ifndef MSRDAILYSKELETONINSTANCE_H
#define MSRDAILYSKELETONINSTANCE_H

#include "dataset/DataInstance.h"
#include "types/SkeletonFrame.h"
#include <fstream>

using namespace std;

namespace dai {

class MSRDailySkeletonInstance : public DataInstance
{
public:
    explicit MSRDailySkeletonInstance(const InstanceInfo& info);
    virtual ~MSRDailySkeletonInstance();
    bool is_open() const override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(QHashDataFrames& output) override;

private:
    static SkeletonJoint::JointType staticMap[20];

    ifstream         m_file;
    int              m_nJoints;
};

} // End of namespace

#endif // MSRDAILYSKELETONINSTANCE_H
