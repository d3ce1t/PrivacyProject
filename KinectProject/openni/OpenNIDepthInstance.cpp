#include "OpenNIDepthInstance.h"
#include "types/DataInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIDepthInstance::OpenNIDepthInstance()
{
    this->m_type = dai::INSTANCE_DEPTH;
    this->m_title = "Depth Live Stream";
    m_frameBuffer[0].reset(new DepthFrame(640, 480));
    m_frameBuffer[1].reset(new DepthFrame(640, 480));
    StreamInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
    m_openni = nullptr;
}

OpenNIDepthInstance::~OpenNIDepthInstance()
{
    closeInstance();
    m_openni = nullptr;
}

void OpenNIDepthInstance::setOutputFile(QString file)
{
    m_outputFile = file;
}

bool OpenNIDepthInstance::is_open() const
{
    return m_openni != nullptr;
}

void OpenNIDepthInstance::openInstance()
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
                    throw 8;
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

void OpenNIDepthInstance::closeInstance()
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

void OpenNIDepthInstance::restartInstance()
{

}

void OpenNIDepthInstance::nextFrame(DepthFrame &frame)
{
    // Read Data from OpenNI
    DepthFrame& depthFrame = (DepthFrame&) frame;
    depthFrame = m_openni->readDepthFrame(); // copy

    if (m_of.isOpen()) {
        depthFrame.write(m_of);
    }
}

} // End namespace
