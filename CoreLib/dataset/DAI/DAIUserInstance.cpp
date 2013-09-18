#include "DAIUserInstance.h"
#include <iostream>

namespace dai {

DAIUserInstance::DAIUserInstance(const InstanceInfo& info)
    : DataInstance(info)
{
    m_frameBuffer[0].reset(new UserFrame(640, 480));
    m_frameBuffer[1].reset(new UserFrame(640, 480));
    DataInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
    m_width = 0;
    m_height = 0;
}

DAIUserInstance::~DAIUserInstance()
{
    m_width = 0;
    m_height = 0;
    closeInstance();
}

bool DAIUserInstance::is_open() const
{
    return m_file.is_open();
}

void DAIUserInstance::openInstance()
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

void DAIUserInstance::closeInstance()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

void DAIUserInstance::restartInstance()
{
    if (m_file.is_open()) {
        m_file.seekg(12, ios_base::beg);
    }
}

void DAIUserInstance::nextFrame(UserFrame &frame)
{
    // Read Data from File
    uint8_t* ptrImg = (uint8_t*) frame.getDataPtr();
    m_file.read( (char *) ptrImg, frame.getWidth() * frame.getHeight() * sizeof(uint8_t) );
}

} // End Namespace