#include "OpenNIColorInstance.h"
#include "dataset/DataInstance.h"
#include <exception>
#include <iostream>
#include <QDebug>

using namespace std;

namespace dai {

OpenNIColorInstance::OpenNIColorInstance()
    : m_currentFrame(640, 480)
{
    this->m_type = StreamInstance::Color;
    this->m_title = "Color Live Stream";
    m_openni = OpenNIRuntime::getInstance();
}

OpenNIColorInstance::~OpenNIColorInstance()
{
    m_openni->releaseInstance();
}

void OpenNIColorInstance::setOutputFile(QString file)
{
    m_outputFile = file;
}

void OpenNIColorInstance::open()
{
    m_frameIndex = 0;

    try {
        if (!m_of.is_open() && !m_outputFile.isEmpty())
        {
            m_of.open(m_outputFile.toStdString().c_str(), ios::out|ios::binary);

            if (!m_of.is_open()) {
                cerr << "Error opening file" << endl;
                throw 7;
            }

            int width = m_currentFrame.getWidth();
            int height = m_currentFrame.getHeight();
            int numFrames = 0;

            m_of.seekp(0, ios_base::beg);
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

void OpenNIColorInstance::close()
{
    try {
        if (m_of.is_open()) {
            m_of.seekp(0, ios_base::beg);
            m_of.write( (char*) &m_frameIndex, sizeof(m_frameIndex) );
            m_of.close();
        }
    }
    catch (std::exception& ex)
    {
        printf("Error\n");
    }
}

bool OpenNIColorInstance::hasNext() const
{
    return true;
}

const ColorFrame &OpenNIColorInstance::nextFrame()
{
    openni::VideoFrameRef colorFrame = m_openni->readColorFrame();

    // RGB Frame
    if ( colorFrame.isValid())
    {
        m_currentFrame.setIndex(m_frameIndex);

        const openni::RGB888Pixel* pImageRow = (const openni::RGB888Pixel*) colorFrame.getData();
        int rowSize = colorFrame.getStrideInBytes() / sizeof(openni::RGB888Pixel);

        for (int y = 0; y < colorFrame.getHeight(); ++y)
        {
            const openni::RGB888Pixel* pImage = pImageRow;

            for (int x = 0; x < colorFrame.getWidth(); ++x, ++pImage)
            {
                RGBAColor color;
                color.red = pImage->r / 255.0f;
                color.green = pImage->g / 255.0f;
                color.blue = pImage->b / 255.0f;
                color.alpha = 1.0;
                m_currentFrame.setItem(y, x, color);
            }

            pImageRow += rowSize;
        }

        if (m_of.is_open()) {
            m_currentFrame.write(m_of);
        }
    }

    m_frameIndex++;
    return m_currentFrame;
}

} // End namespace
