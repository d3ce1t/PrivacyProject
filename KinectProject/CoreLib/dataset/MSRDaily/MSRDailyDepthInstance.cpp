#include "MSRDailyDepthInstance.h"
#include "types/DepthFrame.h"
#include "Utils.h"
#include <cstddef>
#include <math.h>
#include <QDebug>
#include "dataset/DatasetMetadata.h"

using namespace std;

namespace dai {

MSRDailyDepthInstance::MSRDailyDepthInstance(const InstanceInfo &info)
    : DataInstance(info)
{
    m_frameBuffer = make_shared<DepthFrame>(320, 240);
    m_width = 0;
    m_height = 0;
}

MSRDailyDepthInstance::~MSRDailyDepthInstance()
{
    m_width = 0;
    m_height = 0;
    closeInstance();
}

bool MSRDailyDepthInstance::is_open() const
{
    return m_file.is_open();
}

bool MSRDailyDepthInstance::openInstance()
{
    bool result = false;
    QString instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Depth);

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

void MSRDailyDepthInstance::closeInstance()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

void MSRDailyDepthInstance::restartInstance()
{
    if (m_file.is_open()) {
        m_file.seekg(12, ios_base::beg);
    }
}

QList<shared_ptr<DataFrame>> MSRDailyDepthInstance::nextFrames()
{
    QList<shared_ptr<DataFrame>> result;

    // Read Data from File
    m_file.read( (char *) m_readBuffer, sizeof(m_readBuffer) );

    for (int y=0; y<m_height; ++y) {
        for (int x=0; x<m_width; ++x)
        {
            // Kinect SDK provide depth values between 0 and 4000 in mm.
            float value = m_readBuffer[y].depthRow[x] / 1000.0f; // I want meters
            //value = normalise<int32_t>(m_readBuffer[y].depthRow[x], 0, 4000, 0, 1);
            m_frameBuffer->setItem(y, x, value);
        }
    }

    result.append(m_frameBuffer);
    return result;
}

} // End Namespace
