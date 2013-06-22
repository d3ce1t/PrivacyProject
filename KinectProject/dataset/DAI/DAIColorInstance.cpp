#include "DAIColorInstance.h"

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
    close();
}

bool DAIColorInstance::is_open() const
{
    return m_file.is_open();
}

void DAIColorInstance::open()
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

        m_frameIndex = 0;
    }
}

void DAIColorInstance::close()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

void DAIColorInstance::restart()
{
    if (m_file.is_open()) {
        m_frameIndex = 0;
        m_file.seekg(12, ios_base::beg);
    }
}

void DAIColorInstance::nextFrame(DataFrame &frame)
{
    // Read Data from File
    ColorFrame& colorFrame = (ColorFrame&) frame;
    BinaryColorFrame tempFrame[480];
    m_file.read( (char *) tempFrame, sizeof(tempFrame) );

    for (int y=0; y<m_height; ++y)
    {
        for (int x=0; x<m_width; ++x)
        {
            //Loaded depths are already normalised because I did it when saved
            colorFrame.setItem(y, x, tempFrame[y].colorRow[x]);
        }
    }
}

ColorFrame& DAIColorInstance::frame()
{
    return (ColorFrame&) DataInstance::frame();
}

} // End namespace
