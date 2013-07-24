#include "OpenNIUserInstance.h"
#include "dataset/DataInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIUserInstance::OpenNIUserInstance()
{
    this->m_type = StreamInstance::Color;
    this->m_title = "Color Live Stream";
    m_frameBuffer[0].reset(new ColorFrame(640, 480));
    m_frameBuffer[1].reset(new ColorFrame(640, 480));
    StreamInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
    m_openni = nullptr;
}

OpenNIUserInstance::~OpenNIUserInstance()
{
    closeInstance();
    m_openni = nullptr;
}

void OpenNIUserInstance::setOutputFile(QString file)
{
    m_outputFile = file;
}

bool OpenNIUserInstance::is_open() const
{
    return m_openni != nullptr;
}

void OpenNIUserInstance::openInstance()
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

void OpenNIUserInstance::closeInstance()
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

void OpenNIUserInstance::restartInstance()
{

}

void OpenNIUserInstance::nextFrame(DataFrame &frame)
{
    // Read Data from OpenNI
    ColorFrame& colorFrame = (ColorFrame&) frame;
    openni::VideoFrameRef oniColorFrame = m_openni->readColorFrame();

    // RGB Frame
    if ( oniColorFrame.isValid())
    {
        const openni::RGB888Pixel* pImageRow = (const openni::RGB888Pixel*) oniColorFrame.getData();
        int rowSize = oniColorFrame.getStrideInBytes() / sizeof(openni::RGB888Pixel);

        for (int y = 0; y < oniColorFrame.getHeight(); ++y)
        {
            const openni::RGB888Pixel* pImage = pImageRow;

            for (int x = 0; x < oniColorFrame.getWidth(); ++x, ++pImage)
            {
                RGBColor color;
                color.red = pImage->r;
                color.green = pImage->g;
                color.blue = pImage->b;
                colorFrame.setItem(y, x, color);
            }

            pImageRow += rowSize;
        }

        if (m_of.isOpen()) {
            colorFrame.write(m_of);
        }
    }
}

} // End namespace
