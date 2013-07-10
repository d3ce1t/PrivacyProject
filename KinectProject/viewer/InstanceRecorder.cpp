#include "InstanceRecorder.h"
#include <QDebug>
#include "types/DepthFrame.h"
#include "types/ColorFrame.h"
#include "types/UserFrame.h"
#include <iostream>

namespace dai {

InstanceRecorder::InstanceRecorder()
{
    m_of.setFileName("/files/capture/capture.user");
    m_of.open(QIODevice::WriteOnly | QIODevice::Truncate);

    if (!m_of.isOpen()) {
        std::cerr << "Error opening file" << std::endl;
        throw 1;
    }

    m_lastFrame = 0;

    int width = 640;
    int height = 480;
    int numFrames = 0;

    m_of.seek(0);
    m_of.write( (char*) &numFrames, sizeof(numFrames) );
    m_of.write( (char*) &width, sizeof(width) );
    m_of.write( (char*) &height, sizeof(height) );
}

void InstanceRecorder::onPlaybackStart()
{

}

void InstanceRecorder::onPlaybackStop()
{
    if (m_of.isOpen()) {
        m_lastFrame++;
        m_of.seek(0);
        m_of.write( (char*) &m_lastFrame, sizeof(m_lastFrame) );
        m_of.close();
    }

    qDebug() << "File saved sucessfully";
}

void InstanceRecorder::onNewFrame(QList<DataFrame*> dataFrames)
{
    // I do not need to acquiere playback because I'm performing my work
    // in a synchronous way. Acquire and release is for async activities.
    UserFrame* userFrame = (UserFrame*) dataFrames.at(0);
    m_lastFrame = userFrame->getIndex();
    userFrame->write(m_of);
    qDebug() << "Frame " << m_lastFrame << "written";
}

} // End Namespace
