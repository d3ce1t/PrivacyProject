#ifndef MSRDAILYSKELETONINSTANCE_H
#define MSRDAILYSKELETONINSTANCE_H

#include "dataset/DataInstance.h"
#include "types/Skeleton.h"
#include <fstream>

using namespace std;

namespace dai {

class MSRDailySkeletonInstance : public DataInstance
{
public:
    explicit MSRDailySkeletonInstance(const InstanceInfo& info);
    virtual ~MSRDailySkeletonInstance();
    bool is_open() const Q_DECL_OVERRIDE;
    dai::Skeleton& frame() Q_DECL_OVERRIDE;

protected:
    void openInstance() Q_DECL_OVERRIDE;
    void closeInstance() Q_DECL_OVERRIDE;
    void restartInstance() Q_DECL_OVERRIDE;
    void nextFrame(DataFrame& frame) Q_DECL_OVERRIDE;

private:
    static SkeletonJoint::JointType convertIntToType(int value);

    ifstream         m_file;
    int              m_nJoints;
    dai::Skeleton    m_frameBuffer[2];
};

} // End of namespace

#endif // MSRDAILYSKELETONINSTANCE_H
