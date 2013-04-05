#ifndef MSRDAILYACTIVITY3DINSTANCE_H
#define MSRDAILYACTIVITY3DINSTANCE_H

#include <iostream>
#include <fstream>
#include "DataInstance.h"
#include "DepthFrame.h"

using namespace std;

namespace dai {

class MSRDailyActivity3DInstance : public DataInstance
{
public:
    MSRDailyActivity3DInstance(const InstanceInfo& info);
    virtual ~MSRDailyActivity3DInstance();
    void open();
    void close();
    int getTotalFrames();
    bool hasNext();
    DepthFrame* nextFrame();

private:
    ifstream m_file;
    int m_nFrames;
    int m_nColumns;
    int m_nRows;
    int m_frameIndex;
};

}

#endif // MSRDAILYACTIVITY3DINSTANCE_H
