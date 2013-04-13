#ifndef MSRDAILYACTIVITY3DINSTANCE_H
#define MSRDAILYACTIVITY3DINSTANCE_H

#include <iostream>
#include <fstream>
#include "DataInstance.h"
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
    void open();
    void close();
    int getTotalFrames() const;
    bool hasNext() const;
    const DepthFrame& nextFrame();

private:
    ifstream    m_file;
    int         m_nFrames;
    int         m_width;
    int         m_height;
    int         m_frameIndex;
    DepthFrame  m_currentFrame;
};

} // End Namespace

#endif // MSRDAILYACTIVITY3DINSTANCE_H
