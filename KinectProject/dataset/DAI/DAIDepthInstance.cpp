#include "DAIDepthInstance.h"
#include <QDebug>

namespace dai {

DAIDepthInstance::DAIDepthInstance(const InstanceInfo& info)
    : DataInstance(info)
{
    m_frameBuffer[0] = DepthFrame(640, 480);
    m_frameBuffer[1] = DepthFrame(640, 480);
    DataInstance::initFrameBuffer(&m_frameBuffer[0], &m_frameBuffer[1]);
    m_width = 0;
    m_height = 0;
}

DAIDepthInstance::~DAIDepthInstance()
{
    m_width = 0;
    m_height = 0;
    closeInstance();
}

bool DAIDepthInstance::is_open() const
{
    return m_file.is_open();
}

void DAIDepthInstance::openInstance()
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

        if (m_width != 640 || m_height != 480)
            exit(1);
    }
}

void DAIDepthInstance::closeInstance()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

void DAIDepthInstance::restartInstance()
{
    if (m_file.is_open()) {
        m_file.seekg(12, ios_base::beg);
    }
}

void DAIDepthInstance::nextFrame(DataFrame &frame)
{
    // Read Data from File
    DepthFrame& depthFrame = (DepthFrame&) frame;
    m_file.read( (char *) m_readDepthBuffer, sizeof(m_readDepthBuffer) );
    m_file.read( (char *) m_readLabelBuffer, sizeof(m_readLabelBuffer) );

    for (int y=0; y<m_height; ++y) {
        for (int x=0; x<m_width; ++x)
        {
            //Loaded depths are already normalised because I did it when saved
            depthFrame.setItem(y, x, m_readDepthBuffer[y].depthRow[x], m_readLabelBuffer[y].labelRow[x]);
        }
    }
}

DepthFrame& DAIDepthInstance::frame()
{
    return (DepthFrame&) DataInstance::frame();
}

} // End namespace