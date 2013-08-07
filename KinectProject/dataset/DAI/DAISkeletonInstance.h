#ifndef DAI_SKELETON_INSTANCE_H
#define DAI_SKELETON_INSTANCE_H

#include <fstream>
#include "types/DataInstance.h"
#include "types/SkeletonFrame.h"

using namespace std;

namespace dai {

class DAISkeletonInstance : public DataInstance<SkeletonFrame>
{
public:
    explicit DAISkeletonInstance(const InstanceInfo& info);
    virtual ~DAISkeletonInstance();
    bool is_open() const override;

protected:
    void openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(SkeletonFrame& frame) override;

private:
    ifstream    m_file;
    shared_ptr<SkeletonFrame> m_frameBuffer[2];
};

} // End Namespace

#endif // DAI_SKELETON_INSTANCE_H
