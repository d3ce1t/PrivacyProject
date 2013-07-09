#include "DAIColorInstance.h"
#include <QDebug>

namespace dai {

DAIColorInstance::DAIColorInstance(const InstanceInfo& info)
    : DataInstance(info)
{
    m_frameBuffer[0] = ColorFrame(640, 480);
    m_frameBuffer[1] = ColorFrame(640, 480);
    DataInstance::initFrameBuffer(&m_frameBuffer[0], &m_frameBuffer[1]);
    m_width = 0;
    m_height = 0;
}

DAIColorInstance::~DAIColorInstance()
{
    m_width = 0;
    m_height = 0;
    closeInstance();
}

bool DAIColorInstance::is_open() const
{
    return m_file.is_open();
}

void DAIColorInstance::openInstance()
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

void DAIColorInstance::closeInstance()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

void DAIColorInstance::restartInstance()
{
    if (m_file.is_open()) {
        m_file.seekg(12, ios_base::beg);
    }
}

void DAIColorInstance::nextFrame(DataFrame &frame)
{
    // Read Data from File
    ColorFrame& colorFrame = (ColorFrame&) frame;
    m_file.read( (char *) m_readBuffer, sizeof(m_readBuffer) );

    for (int y=0; y<m_height; ++y)
    {
        for (int x=0; x<m_width; ++x)
        {
            colorFrame.setItem(y, x, m_readBuffer[y].colorRow[x]);
        }
    }
}

ColorFrame& DAIColorInstance::frame()
{
    return (ColorFrame&) DataInstance::frame();
}

} // End namespace
