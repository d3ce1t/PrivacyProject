#include "OpenNIDepthInstance.h"
#include "dataset/DataInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIDepthInstance::OpenNIDepthInstance()
{
    this->m_type = StreamInstance::Depth;
    this->m_title = "Depth Live Stream";
    m_frameBuffer[0] = DepthFrame(640, 480);
    m_frameBuffer[1] = DepthFrame(640, 480);
    StreamInstance::initFrameBuffer(&m_frameBuffer[0], &m_frameBuffer[1]);
    m_openni = NULL;
}

OpenNIDepthInstance::~OpenNIDepthInstance()
{
    closeInstance();
    m_openni = NULL;
}

void OpenNIDepthInstance::setOutputFile(QString file)
{
    m_outputFile = file;
}

bool OpenNIDepthInstance::is_open() const
{
    return m_openni != NULL;
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
        m_openni = NULL;

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

void OpenNIDepthInstance::nextFrame(DataFrame &frame)
{
    // Read Data from OpenNI
    DepthFrame& depthFrame = (DepthFrame&) frame;
    nite::UserTrackerFrameRef userTrackerFrame = m_openni->readUserTrackerFrame();

    // Depth Frame
    if (userTrackerFrame.isValid())
    {
        //m_pUserTracker.setSkeletonSmoothingFactor(0.7);
        openni::VideoFrameRef oniDepthFrame = m_openni->readDepthFrame();
        const nite::UserId* pLabel = userTrackerFrame.getUserMap().getPixels();
        const openni::DepthPixel* pDepth = (const openni::DepthPixel*)oniDepthFrame.getData();
        int restOfRow = oniDepthFrame.getStrideInBytes() / sizeof(openni::DepthPixel) - oniDepthFrame.getWidth();
        int height = oniDepthFrame.getHeight();
        int width = oniDepthFrame.getWidth();

        for (int y=0; y<height; ++y)
        {
            for (int x=0; x<width; ++x, ++pLabel)
            {
                // FIX: I assume depth value is between 0 a 10000.
                depthFrame.setItem(y, x, DataInstance::normalise(*pDepth, 0, 10000, 0, 1)/*, *pLabel*/);
                pDepth++;
            }

            // Skip rest of row (in case it exists)
            pDepth += restOfRow;
        }

        if (m_of.isOpen()) {
            depthFrame.write(m_of);
        }
    }
}

DepthFrame& OpenNIDepthInstance::frame()
{
    return (DepthFrame&) StreamInstance::frame();
}

} // End namespace
