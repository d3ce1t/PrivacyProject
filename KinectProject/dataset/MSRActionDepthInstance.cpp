#include "MSRActionDepthInstance.h"
#include "types/DepthFrame.h"
#include <cstddef>
#include <math.h>
#include <stdint.h>
#include <QDebug>

using namespace std;

namespace dai {

MSRActionDepthInstance::MSRActionDepthInstance(const InstanceInfo &info)
    : DataInstance(info), m_currentFrame(1, 1)
{
    m_nFrames = 0;
    m_width = 0;
    m_height = 0;
    m_frameIndex = 0;
}

MSRActionDepthInstance::~MSRActionDepthInstance()
{
    close();
}

void MSRActionDepthInstance::open()
{
    QString instancePath = m_info.getDatasetPath() + "/" + m_info.getFileName();
    m_file.open(instancePath.toStdString().c_str(), ios::in|ios::binary);

    if (!m_file.is_open()) {
        cerr << "Error opening file" << endl;
        return;
    }

    m_file.read((char *) &m_nFrames, 4);
    m_file.read((char *) &m_width, 4);
    m_file.read((char *) &m_height, 4);

    m_currentFrame = DepthFrame(m_width, m_height);
}

void MSRActionDepthInstance::close()
{
    if (m_file.is_open()) {
        m_file.close();
    }

    m_nFrames = 0;
    m_width = 0;
    m_height = 0;
    m_frameIndex = 0;
}

int MSRActionDepthInstance::getTotalFrames()
{
    return m_nFrames;
}

/*int MSRDailyDepthInstance::getResolutionX()
{
    return m_width;
}

int MSRDailyDepthInstance::getResolutionY()
{
    return m_height;
}*/

bool MSRActionDepthInstance::hasNext()
{
    if (m_frameIndex < m_nFrames || m_playLoop)
        return true;

    return false;
}

const DepthFrame &MSRActionDepthInstance::nextFrame()
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
        int* tempRow = new int[m_width];

        for(int r=0; r<m_height; r++)
        {
            m_file.read((char *) tempRow, 4*m_width);

            for(int c=0; c<m_width; c++) {
                m_currentFrame.setItem(r, c, tempRow[c]);
            }
        }

        delete[] tempRow;
        tempRow = NULL;

        m_frameIndex++;
    }
    else {
        close();
    }

    return m_currentFrame;
}

} // End Namespace
