#ifndef MSRDAILYACTIVITY3DINSTANCE_H
#define MSRDAILYACTIVITY3DINSTANCE_H

#include <iostream>
#include <fstream>
#include "DataInstance.h"
#include "types/DepthFrame.h"

using namespace std;

namespace dai {

class MSRDailyDepthInstance : public DataInstance
{
public:
    explicit MSRDailyDepthInstance(const InstanceInfo& info);
    virtual ~MSRDailyDepthInstance();
    void open();
    void close();
    int getTotalFrames();
    /*int getResolutionX();
    int getResolutionY();*/
    bool hasNext();
    const DepthFrame& nextFrame();

private:
    ifstream    m_file;
    int         m_nFrames;
    int         m_width;
    int         m_height;
    int         m_frameIndex;
    DepthFrame  m_currentFrame;
};

}

#endif // MSRDAILYACTIVITY3DINSTANCE_H
