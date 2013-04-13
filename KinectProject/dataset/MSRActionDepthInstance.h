#ifndef MSR_ACTION3D_INSTANCE_H
#define MSR_ACTION3D_INSTANCE_H

#include <iostream>
#include <fstream>
#include "DataInstance.h"
#include "types/DepthFrame.h"
#include <stdint.h>

using namespace std;

namespace dai {

class MSRActionDepthInstance : public DataInstance
{
public:

    struct BinaryDepthFrame {
        int32_t depthRow[320];
    };

    explicit MSRActionDepthInstance(const InstanceInfo& info);
    virtual ~MSRActionDepthInstance();
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

#endif // MSR_ACTION3D_INSTANCE_H
