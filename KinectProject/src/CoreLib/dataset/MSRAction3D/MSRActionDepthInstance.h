#ifndef MSR_ACTION3D_INSTANCE_H
#define MSR_ACTION3D_INSTANCE_H

#include <iostream>
#include <fstream>
#include "dataset/DataInstance.h"
#include "types/DepthFrame.h"
#include <stdint.h>
#include <QMutex>

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
    bool is_open() const override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(QHashDataFrames& output) override;

private:
    static uint16_t _distances_table[2048];
    static bool     _initialised;
    static QMutex   _mutex;

    ifstream    m_file;
    int         m_width;
    int         m_height;
    BinaryDepthFrame m_readBuffer[240]; // I know MSR Action 3D depth is 320 x 240
};

} // End Namespace

#endif // MSR_ACTION3D_INSTANCE_H
