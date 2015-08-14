#include "MSRActionDepthInstance.h"
#include "types/DepthFrame.h"
#include "Utils.h"
#include "dataset/DatasetMetadata.h"
#include <cmath>

namespace dai {

uint16_t MSRActionDepthInstance::_distances_table[2048];
bool MSRActionDepthInstance::_initialised = false;
QMutex MSRActionDepthInstance::_mutex;

MSRActionDepthInstance::MSRActionDepthInstance(const InstanceInfo &info)
    : DataInstance(info, DataFrame::Depth, 320, 240)
{
    // Compute distances look up table (shared between all instances)
    _mutex.lock();
    if (!_initialised) {
        for (int i=0; i<2048; ++i) {
            _distances_table[i] = 0.1236 * tan(float(i) / 2842.5f + 1.1863) * 1000;
        }
        _initialised = true;
    }
    _mutex.unlock();

    m_width = 0;
    m_height = 0;
}

MSRActionDepthInstance::~MSRActionDepthInstance()
{
    m_width = 0;
    m_height = 0;
    closeInstance();
}

bool MSRActionDepthInstance::is_open() const
{
    return m_file.is_open();
}

bool MSRActionDepthInstance::openInstance()
{
    bool result = false;
    QString datasetPath = m_info.parent().getPath();
    QString instancePath = datasetPath + "/" + m_info.getFileName(DataFrame::Depth);

    if (!m_file.is_open())
    {
        m_file.open(instancePath.toStdString().c_str(), ios::in|ios::binary);

        if (m_file.is_open())
        {
            m_file.seekg(0, ios_base::beg);
            m_file.read((char *) &m_nFrames, 4);
            m_file.read((char *) &m_width, 4);
            m_file.read((char *) &m_height, 4);

            if (m_width != 320 || m_height != 240)
                exit(1);

            result = true;
        }
    }

    return result;
}

void MSRActionDepthInstance::closeInstance()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

void MSRActionDepthInstance::restartInstance()
{
    if (m_file.is_open()) {
        m_file.seekg(12, ios_base::beg);
    }
}

void MSRActionDepthInstance::nextFrame(QHashDataFrames &output)
{
    Q_ASSERT(output.size() > 0);
    shared_ptr<DepthFrame> depthFrame = static_pointer_cast<DepthFrame>(output.value(DataFrame::Depth));
    depthFrame->setDistanceUnits(dai::DISTANCE_MILIMETERS);

    // Read Data from File
    m_file.read( (char *) m_readBuffer, sizeof(m_readBuffer) );

    for (int y=0; y<m_height; ++y) {
        for (int x=0; x<m_width; ++x)
        {
            // MSR Action3d data is captured from a Kinect like device
            // I assume data is in raw. So I have to convert it to milimeters
            uint16_t rawValue = m_readBuffer[y].depthRow[x];
            uint16_t value = 0;

            if (rawValue > 0 && rawValue < 2047)
                value = _distances_table[rawValue];

            /* http://openkinect.org/wiki/Imaging_Information
             * x = (i - w / 2) * (z + minDistance) * scaleFactor
             * y = (j - h / 2) * (z + minDistance) * scaleFactor
             * z = z
             * Where
             * minDistance = -10
             * scaleFactor = .0021
             */

            /*if (value != 0) {
                value = 2.0 + normalise<float>(m_readBuffer[y].depthRow[x], 290, 649, 0, 0.9f);
            }*/

            depthFrame->setItem(y, x, value);
        }
    }
}

} // End Namespace
