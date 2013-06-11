#include "OpenNIDepthInstance.h"
#include "dataset/DataInstance.h"
#include <exception>
#include <iostream>
#include <QDebug>

using namespace std;

namespace dai {

OpenNIDepthInstance::OpenNIDepthInstance()
    : m_currentFrame(640, 480)
{
    this->m_type = StreamInstance::Depth;
    this->m_title = "Depth Live Stream";
}

OpenNIDepthInstance::~OpenNIDepthInstance()
{
    //nite::NiTE::shutdown();
    //openni::OpenNI::shutdown();
}

void OpenNIDepthInstance::setOutputFile(QString file)
{
    m_outputFile = file;
}

void OpenNIDepthInstance::open()
{
    const char* deviceURI = openni::ANY_DEVICE;
    m_frameIndex = 0;

    try {
        if (openni::OpenNI::initialize() != openni::STATUS_OK)
            throw 1;

        if (m_device.open(deviceURI) != openni::STATUS_OK)
            throw 2;

        if (nite::NiTE::initialize() != nite::STATUS_OK)
            throw 3;

        /*if (m_colorStream.create(m_device, openni::SENSOR_COLOR) != openni::STATUS_OK)
            throw 4;

        if (m_colorStream.start() != openni::STATUS_OK)
            throw 5;*/

        if (m_pUserTracker.create(&m_device) != nite::STATUS_OK) {
            printf("algo fallo\n");
            throw 6;
        }

        if (!m_pUserTracker.isValid() /*|| !m_colorStream.isValid()*/)
            throw 7;

        if (!m_of.is_open() && !m_outputFile.isEmpty())
        {
            m_of.open(m_outputFile.toStdString().c_str(), ios::out|ios::binary);

            if (!m_of.is_open()) {
                cerr << "Error opening file" << endl;
                throw 8;
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
        nite::NiTE::shutdown();
        openni::OpenNI::shutdown();
        throw ex;
    }
}

void OpenNIDepthInstance::close()
{
    try {
        if (m_of.is_open()) {
            m_of.seekp(0, ios_base::beg);
            m_of.write( (char*) &m_frameIndex, sizeof(m_frameIndex) );
            m_of.close();
        }

        //m_device.close();
        //nite::NiTE::shutdown();
        //openni::OpenNI::shutdown();
    }
    catch (std::exception& ex)
    {
        printf("Error\n");
    }
}

bool OpenNIDepthInstance::hasNext() const
{
    return true;
}

const DepthFrame& OpenNIDepthInstance::nextFrame()
{
    // Read Depth Frame
    m_currentFrame.setIndex(m_frameIndex);
    nite::UserTrackerFrameRef userTrackerFrame;

    //m_pUserTracker.setSkeletonSmoothingFactor(0.7);

    if (m_pUserTracker.readFrame(&userTrackerFrame) != nite::STATUS_OK) {
        throw 1;
    }

    //m_colorStream.readFrame(&m_colorFrame);
    videoMode = userTrackerFrame.getDepthFrame().getVideoMode();

    openni::VideoFrameRef frameRef = userTrackerFrame.getDepthFrame();
    const nite::UserMap& userLabels = userTrackerFrame.getUserMap();
    const nite::UserId* pLabel = userLabels.getPixels();
    const openni::DepthPixel* pDepth = (const openni::DepthPixel*)frameRef.getData();
    int restOfRow = frameRef.getStrideInBytes() / sizeof(openni::DepthPixel) - frameRef.getWidth();
    int height = frameRef.getHeight();
    int width = frameRef.getWidth();

    for (int y=0; y<height; ++y)
    {
        for (int x=0; x<width; ++x, ++pLabel)
        {
            // FIX: I assume depth value is between 0 a 10000.
            m_currentFrame.setItem(y, x, DataInstance::normalise(*pDepth, 0, 10000, 0, 1), *pLabel);
            pDepth++;
        }

        // Skip rest of row (in case it exists)
        pDepth += restOfRow;
    }

    if (m_of.is_open()) {
        m_currentFrame.write(m_of);
    }

    m_frameIndex++;
    return m_currentFrame;
}

} // End namespace
