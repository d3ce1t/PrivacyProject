#include "MSRActionDepthInstance.h"
#include "types/DepthFrame.h"
#include "Utils.h"
#include "dataset/DatasetMetadata.h"

namespace dai {

MSRActionDepthInstance::MSRActionDepthInstance(const InstanceInfo &info)
    : DataInstance(info)
{
    m_frameBuffer[0].reset(new DepthFrame(320, 240));
    m_frameBuffer[1].reset(new DepthFrame(320, 240));
    DataInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
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

void MSRActionDepthInstance::openInstance()
{
    QString datasetPath = m_info.parent().getPath();
    QString instancePath = datasetPath + "/" + m_info.getFileName();

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

void MSRActionDepthInstance::nextFrame(DepthFrame &frame)
{
    // Read Data from File
    m_file.read( (char *) m_readBuffer, sizeof(m_readBuffer) );

    for (int y=0; y<m_height; ++y) {
        for (int x=0; x<m_width; ++x)
        {
            float value = m_readBuffer[y].depthRow[x];

            if (value != 0) {
                value = 2.0 + normalise<float>(m_readBuffer[y].depthRow[x], 290, 649, 0, 0.9);
            }

            frame.setItem(y, x, value);
        }
    }
}

} // End Namespace
