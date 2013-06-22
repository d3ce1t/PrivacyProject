#include "MSRDailyDepthInstance.h"
#include "types/DepthFrame.h"
#include <cstddef>
#include <math.h>
#include <QDebug>

using namespace std;

namespace dai {

MSRDailyDepthInstance::MSRDailyDepthInstance(const InstanceInfo &info)
    : DataInstance(info)
{
    m_frameBuffer[0] = DepthFrame(320, 240);
    m_frameBuffer[1] = DepthFrame(320, 240);
    DataInstance::initFrameBuffer(&m_frameBuffer[0], &m_frameBuffer[1]);
    m_width = 0;
    m_height = 0;
}

MSRDailyDepthInstance::~MSRDailyDepthInstance()
{
    m_width = 0;
    m_height = 0;
    close();
}

bool MSRDailyDepthInstance::is_open() const
{
    return m_file.is_open();
}

void MSRDailyDepthInstance::open()
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

void MSRDailyDepthInstance::close()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

void MSRDailyDepthInstance::restart()
{
    if (m_file.is_open()) {
        m_frameIndex = 0;
        m_file.seekg(12, ios_base::beg);
    }
}

void MSRDailyDepthInstance::nextFrame(DataFrame &frame)
{
    // Read Data from File
    DepthFrame& depthFrame = (DepthFrame&) frame;
    BinaryDepthFrame tempFrame[240]; // I know MSR Daily Activity 3D depth is 320 x 240
    m_file.read( (char *) tempFrame, sizeof(tempFrame) );

    for (int y=0; y<m_height; ++y) {
        for (int x=0; x<m_width; ++x)
        {
            // Normalise: Kinect SDK provide depth values between 0 and 4000 in mm.
            depthFrame.setItem(y, x, DataInstance::normalise(tempFrame[y].depthRow[x], 0, 4000, 0, 1));
        }
    }
}

DepthFrame& MSRDailyDepthInstance::frame()
{
    return (DepthFrame&) DataInstance::frame();
}

} // End Namespace
