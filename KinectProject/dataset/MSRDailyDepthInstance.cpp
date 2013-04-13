#include "MSRDailyDepthInstance.h"
#include "types/DepthFrame.h"
#include <cstddef>
#include <math.h>
#include <QDebug>

using namespace std;

namespace dai {

MSRDailyDepthInstance::MSRDailyDepthInstance(const InstanceInfo &info)
    : DataInstance(info), m_currentFrame(320, 240)
{
    m_nFrames = 0;
    m_width = 0;
    m_height = 0;
    m_frameIndex = 0;
}

MSRDailyDepthInstance::~MSRDailyDepthInstance()
{
    close();
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
    }
}

void MSRDailyDepthInstance::close()
{
    if (m_file.is_open()) {
        m_file.close();
    }

    m_nFrames = 0;
    m_width = 0;
    m_height = 0;
    m_frameIndex = 0;
}

int MSRDailyDepthInstance::getTotalFrames()
{
    return m_nFrames;
}

bool MSRDailyDepthInstance::hasNext()
{
    if (m_file.is_open() && (m_frameIndex < m_nFrames || m_playLoop))
        return true;

    return false;
}

const DepthFrame &MSRDailyDepthInstance::nextFrame()
{
    if (m_playLoop) {
        if (m_frameIndex == m_nFrames) {
            m_frameIndex = 0;
            m_file.seekg(12, ios_base::beg);
        }
    }

    if (m_frameIndex < m_nFrames)
    {
        m_currentFrame.setIndex(m_frameIndex);

        // Read Data from File
        BinaryDepthFrame tempFrame[240]; // I know MSR Daily Activity 3D depth is 320 x 240
        m_file.read( (char *) tempFrame, sizeof(tempFrame) );
        //float *data = m_currentFrame.getDataPtr();

        for (int y=0; y<m_height; ++y) {
            for (int x=0; x<m_width; ++x)
            {
                // Normalise: Kinect SDK provide depth values between 0 and 4000 in mm.
                m_currentFrame.setItem(y, x, DataInstance::normalise(tempFrame[y].depthRow[x], 0, 4000, 0, 1));
            }
        }

        /*for (int r=0; r<m_height; r++)
        {
            memcpy(data, tempFrame[r].depthRow, m_width * sizeof(float));
            data += m_width;
        }*/

        m_frameIndex++;
    }
    else {
        close();
    }



    return m_currentFrame;
}

} // End Namespace
