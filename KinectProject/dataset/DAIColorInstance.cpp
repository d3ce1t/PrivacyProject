#include "DAIColorInstance.h"
#include <QDebug>

namespace dai {

DAIColorInstance::DAIColorInstance(const InstanceInfo& info)
    : DataInstance(info), m_currentFrame(640, 480)
{
    m_nFrames = 0;
    m_width = 0;
    m_height = 0;
    m_frameIndex = 0;
}

DAIColorInstance::~DAIColorInstance()
{
    close();
}

void DAIColorInstance::open()
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

        qDebug() << m_nFrames << m_width << m_height;
    }
}

void DAIColorInstance::close()
{
    if (m_file.is_open()) {
        m_file.close();
    }

    m_nFrames = 0;
    m_width = 0;
    m_height = 0;
    m_frameIndex = 0;
}

int DAIColorInstance::getTotalFrames() const
{
    return m_nFrames;
}

bool DAIColorInstance::hasNext() const
{
    if (m_file.is_open() && (m_frameIndex < m_nFrames || m_playLoop))
        return true;

    return false;
}

const ColorFrame& DAIColorInstance::nextFrame()
{
    if (m_playLoop) {
        if (m_frameIndex == m_nFrames) {
            m_frameIndex = 0;
            m_file.seekg(12, ios_base::beg);
        }
    }

    if (m_frameIndex < m_nFrames)
    {
        m_currentFrame.setIndex(m_frameIndex);

        // Read Data from File
        BinaryColorFrame tempFrame[480];
        m_file.read( (char *) tempFrame, sizeof(tempFrame) );

        for (int y=0; y<m_height; ++y) {
            for (int x=0; x<m_width; ++x)
            {
                //Loaded depths are already normalised because I did it when saved
                m_currentFrame.setItem(y, x, tempFrame[y].colorRow[x]);
            }
        }

        m_frameIndex++;
    }
    else {
        close();
    }

    return m_currentFrame;
}

ColorFrame& DAIColorInstance::frame()
{
    return m_currentFrame;
}


} // End namespace
