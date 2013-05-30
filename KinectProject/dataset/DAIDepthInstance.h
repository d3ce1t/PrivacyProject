#ifndef DAIDEPTHINSTANCE_H
#define DAIDEPTHINSTANCE_H

#include <iostream>
#include <fstream>
#include "DataInstance.h"
#include "../types/DepthFrame.h"
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

#endif // DAIDEPTHINSTANCE_H
