#include "MSRActionDepthInstance.h"
#include "types/DepthFrame.h"
#include <cstddef>
#include <math.h>
#include <stdint.h>
#include <QDebug>
#include <limits>

namespace dai {

MSRActionDepthInstance::MSRActionDepthInstance(const InstanceInfo &info)
    : DataInstance(info)
{
    m_frameBuffer[0] = DepthFrame(320, 240);
    m_frameBuffer[1] = DepthFrame(320, 240);
    DataInstance::initFrameBuffer(&m_frameBuffer[0], &m_frameBuffer[1]);
    m_width = 0;
    m_height = 0;
}

MSRActionDepthInstance::~MSRActionDepthInstance()
{
    m_width = 0;
    m_height = 0;
    close();
}

bool MSRActionDepthInstance::is_open() const
{
    return m_file.is_open();
}

void MSRActionDepthInstance::open()
{
    QString instancePath = m_info.getDatasetPath() + "/" + m_info.getFileName();

    if (!m_file.is_open())
    {
        m_file.open(instancePath.toStdString().c_str(), ios::in|ios::binary);

        if (!m_file.is_open()) {
            cerr << "Error opening file" << endl;
            return;
        }

        m_file.seekg(0, ios_base::beg);

        m_file.read((char *) &m_nFrames, 4);
        m_file.read((char *) &m_width, 4);
        m_file.read((char *) &m_height, 4);

        if (m_width != 320 || m_height != 240)
            exit(1);

        m_frameIndex = 0;
    }
}

void MSRActionDepthInstance::close()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

void MSRActionDepthInstance::restart()
{
    if (m_file.is_open()) {
        m_frameIndex = 0;
        m_file.seekg(12, ios_base::beg);
    }
}

void MSRActionDepthInstance::nextFrame(DataFrame &frame)
{
    // Read Data from File
    DepthFrame& depthFrame = (DepthFrame&) frame;
    BinaryDepthFrame tempFrame[240]; // I know MSR Action 3D depth is 320 x 240
    m_file.read( (char *) tempFrame, sizeof(tempFrame) );
    //float *data = m_currentFrame.getDataPtr();

    for (int y=0; y<m_height; ++y) {
        for (int x=0; x<m_width; ++x)
        {
            // FIX: I assume depth value is between 0 a 10000. But I'm not sure. This dataset
            // is recorder using a kinect like device, but I dont'know which.
            depthFrame.setItem(y, x, DataInstance::normalise(tempFrame[y].depthRow[x], 0, 10000, 0, 1));
            // m_currentFrame.setItem(y, x, tempFrame[y].depthRow[x]);
        }
    }

    /*for (int r=0; r<m_height; r++)
      {
          memcpy(data, tempFrame[r].depthRow, m_width * sizeof(float));
          data += m_width;
      }*/
}

DepthFrame& MSRActionDepthInstance::frame()
{
    return (DepthFrame&) DataInstance::frame();
}

} // End Namespace
