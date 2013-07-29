#include "OpenNISkeletonInstance.h"
#include "dataset/DataInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNISkeletonInstance::OpenNISkeletonInstance()
{
    this->m_type = StreamInstance::Skeleton;
    this->m_title = "Skeleton Live Stream";
    m_frameBuffer[0].reset(new SkeletonFrame);
    m_frameBuffer[1].reset(new SkeletonFrame);
    StreamInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
    m_openni = nullptr;
}

OpenNISkeletonInstance::~OpenNISkeletonInstance()
{
    closeInstance();
    m_openni = nullptr;
}

void OpenNISkeletonInstance::setOutputFile(QString file)
{
    m_outputFile = file;
}

bool OpenNISkeletonInstance::is_open() const
{
    return m_openni != nullptr;
}

void OpenNISkeletonInstance::openInstance()
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

                int numFrames = 0;

                m_ts.setDevice(&m_of);
                m_of.seek(0);
                m_of.write( (char*) &numFrames, sizeof(numFrames) );
            }
        }
        catch (int ex)
        {
            printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
            throw ex;
        }
    }
}

void OpenNISkeletonInstance::closeInstance()
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

void OpenNISkeletonInstance::restartInstance()
{

}

void OpenNISkeletonInstance::nextFrame(DataFrame &frame)
{
    // Read Data from OpenNI
    SkeletonFrame& skeletonFrame = static_cast<SkeletonFrame&>(frame);
    skeletonFrame = m_openni->readSkeletonFrame(); // copy

    if (m_of.isOpen()) {
        skeletonFrame.write(m_of);
    }
}

} // End Namespace
