#ifndef MSR_ACTION3D_INSTANCE_H
#define MSR_ACTION3D_INSTANCE_H

#include <iostream>
#include <fstream>
#include "dataset/DataInstance.h"
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
    bool is_open() const Q_DECL_OVERRIDE;
    void open() Q_DECL_OVERRIDE;
    void close() Q_DECL_OVERRIDE;
    DepthFrame& frame() Q_DECL_OVERRIDE;

protected:
    void nextFrame(DataFrame& frame) Q_DECL_OVERRIDE;
    void restart() Q_DECL_OVERRIDE;

private:
    ifstream    m_file;
    int         m_width;
    int         m_height;
    DepthFrame  m_frameBuffer[2];
    BinaryDepthFrame m_readBuffer[240]; // I know MSR Action 3D depth is 320 x 240
};

} // End Namespace

#endif // MSR_ACTION3D_INSTANCE_H
