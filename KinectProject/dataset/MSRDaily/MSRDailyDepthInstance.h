#ifndef MSRDAILYACTIVITY3DINSTANCE_H
#define MSRDAILYACTIVITY3DINSTANCE_H

#include <iostream>
#include <fstream>
#include "dataset/DataInstance.h"
#include "types/DepthFrame.h"
#include <stdint.h>

using namespace std;

namespace dai {

class MSRDailyDepthInstance : public DataInstance
{
public:
    struct BinaryDepthFrame {
        int32_t depthRow[320];
        uint8_t skelId[320];
    };

    explicit MSRDailyDepthInstance(const InstanceInfo& info);
    virtual ~MSRDailyDepthInstance();
    bool is_open() const override;

protected:
    void openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(DataFrame& frame) override;

private:
    ifstream    m_file;
    int         m_width;
    int         m_height;
    shared_ptr<DataFrame>  m_frameBuffer[2];
    BinaryDepthFrame m_readBuffer[240]; // I know MSR Daily Activity 3D depth is 320 x 240
};

} // End Namespace

#endif // MSRDAILYACTIVITY3DINSTANCE_H
