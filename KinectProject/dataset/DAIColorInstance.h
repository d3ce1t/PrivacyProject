#ifndef DAI_COLOR_INSTANCE_H
#define DAI_COLOR_INSTANCE_H

#include <iostream>
#include <fstream>
#include "DataInstance.h"
#include "../types/ColorFrame.h"
#include <stdint.h>

using namespace std;

namespace dai {

class DAIColorInstance : public DataInstance
{
public:

    struct BinaryColorFrame {
        RGBAColor colorRow[640];
    };

    explicit DAIColorInstance(const InstanceInfo& info);
    virtual ~DAIColorInstance();
    void open();
    void close();
    int getTotalFrames() const;
    bool hasNext() const;
    const ColorFrame& nextFrame();
    ColorFrame& frame();

private:
    ifstream    m_file;
    int         m_nFrames;
    int         m_width;
    int         m_height;
    int         m_frameIndex;
    ColorFrame  m_currentFrame;
};

} // End Namespace

#endif // DAI_COLOR_INSTANCE_H
