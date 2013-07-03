#ifndef DAIDEPTHINSTANCE_H
#define DAIDEPTHINSTANCE_H

#include <iostream>
#include <fstream>
#include "dataset/DataInstance.h"
#include "types/DepthFrame.h"
#include <stdint.h>

using namespace std;

namespace dai {

class DAIDepthInstance : public DataInstance
{
public:
    struct BinaryDepthFrame {
        float depthRow[640];
    };

    struct BinaryLabels {
        short int labelRow[640];
    };

    explicit DAIDepthInstance(const InstanceInfo& info);
    virtual ~DAIDepthInstance();
    bool is_open() const Q_DECL_OVERRIDE;

    DepthFrame& frame() Q_DECL_OVERRIDE;

protected:
    void openInstance() Q_DECL_OVERRIDE;
    void closeInstance() Q_DECL_OVERRIDE;
    void restartInstance() Q_DECL_OVERRIDE;
    void nextFrame(DataFrame& frame) Q_DECL_OVERRIDE;

private:
    ifstream    m_file;
    int         m_width;
    int         m_height;
    DepthFrame  m_frameBuffer[2];
    BinaryDepthFrame m_readDepthBuffer[480];
    BinaryLabels m_readLabelBuffer[480];
};

} // End Namespace

#endif // DAIDEPTHINSTANCE_H
