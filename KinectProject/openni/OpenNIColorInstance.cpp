#include "OpenNIColorInstance.h"
#include "types/DataInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIColorInstance::OpenNIColorInstance()
{
    this->m_type = dai::INSTANCE_COLOR;
    this->m_title = "Color Live Stream";
    m_frameBuffer[0].reset(new ColorFrame(640, 480));
    m_frameBuffer[1].reset(new ColorFrame(640, 480));
    StreamInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
    m_openni = nullptr;
}

OpenNIColorInstance::~OpenNIColorInstance()
{
    closeInstance();
    m_openni = nullptr;
}

void OpenNIColorInstance::setOutputFile(QString file)
{
    m_outputFile = file;
}

bool OpenNIColorInstance::is_open() const
{
    return m_openni != nullptr;
}

void OpenNIColorInstance::openInstance()
{
    if (!is_open())
    {
        m_openni = OpenNIRuntime::getInstance();

        try {
            if (!m_of.isOpen() && !m_outputFile.isEmpty())
            {
                m_of.setFileName(m_outputFile);
                m_of.open(QIODevice::WriteOnly | QIODevice::Truncate);

                if (!m_of.isOpen()) {
                    cerr << "Error opening file" << endl;
                    throw 7;
                }

                int width = 640;
                int height = 480;
                int numFrames = 0;

                m_of.seek(0);
                m_of.write( (char*) &numFrames, sizeof(numFrames) );
                m_of.write( (char*) &width, sizeof(width) );
                m_of.write( (char*) &height, sizeof(height) );
            }
        }
        catch (int ex)
        {
            printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
            throw ex;
        }
    }
}

void OpenNIColorInstance::closeInstance()
{
    if (is_open())
    {
        m_openni->releaseInstance();
        m_openni = nullptr;

        try {
            unsigned int frameIndex = getFrameIndex();
            if (m_of.isOpen()) {
                m_of.seek(0);
                m_of.write( (char*) &frameIndex, sizeof(frameIndex) );
                m_of.close();
            }
        }
        catch (std::exception& ex)
        {
            printf("Error\n");
        }
    }
}

void OpenNIColorInstance::restartInstance()
{

}

void OpenNIColorInstance::nextFrame(ColorFrame &frame)
{
    // Read Data from OpenNI
    ColorFrame& colorFrame = (ColorFrame&) frame;
    colorFrame = m_openni->readColorFrame(); // copy

    if (m_of.isOpen()) {
        colorFrame.write(m_of);
    }
}

} // End namespace
