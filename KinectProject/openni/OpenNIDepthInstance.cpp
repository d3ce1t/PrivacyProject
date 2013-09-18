#include "OpenNIDepthInstance.h"
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
        m_openni->addNewDepthListener(this);

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
        m_openni->removeDepthListener(this);
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
    // Wait until OpenNI provide a new frame
    waitForNewFrame();

    // Read this frame
    QMutexLocker locker(&m_lockFrame);
    const openni::DepthPixel* pDepth = (const openni::DepthPixel*) m_oniDepthFrame.getData();
    frame.setIndex(m_oniDepthFrame.getFrameIndex());

    for (int y=0; y < m_oniDepthFrame.getHeight(); ++y) {
        for (int x=0; x < m_oniDepthFrame.getWidth(); ++x) {
            frame.setItem(y, x, *pDepth / 1000.0f);
            pDepth++;
        }
        // Skip rest of row (in case it exists)
        //pDepth += strideDepth;
    }

    // Stats
    computeStats(frame.getIndex());

    if (m_of.isOpen()) {
        frame.write(m_of);
    }
}

void OpenNIDepthInstance::onNewFrame(openni::VideoStream& stream)
{
    m_lockFrame.lock();

    if (stream.readFrame(&m_oniDepthFrame) != openni::STATUS_OK) {
        throw 1;
    }

    if (!m_oniDepthFrame.isValid()) {
        throw 2;
    }

    int strideDepth = m_oniDepthFrame.getStrideInBytes() / sizeof(openni::DepthPixel) - m_oniDepthFrame.getWidth();

    if (strideDepth > 0) {
        qWarning() << "WARNING: OpenNIRuntime - Not managed depth stride!!!";
        throw 3;
    }

    m_lockFrame.unlock();

    notifyNewFrame();
}

} // End namespace
