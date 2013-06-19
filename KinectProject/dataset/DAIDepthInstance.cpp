#include "DAIDepthInstance.h"
#include <QDebug>
#include "opencv2/opencv.hpp"

namespace dai {

DAIDepthInstance::DAIDepthInstance(const InstanceInfo& info)
    : DataInstance(info), m_currentFrame(640, 480)
{
    m_nFrames = 0;
    m_width = 0;
    m_height = 0;
    m_frameIndex = 0;
}

DAIDepthInstance::~DAIDepthInstance()
{
    close();
}

void DAIDepthInstance::open()
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

void DAIDepthInstance::close()
{
    if (m_file.is_open()) {
        m_file.close();
    }

    m_nFrames = 0;
    m_width = 0;
    m_height = 0;
    m_frameIndex = 0;
}

int DAIDepthInstance::getTotalFrames() const
{
    return m_nFrames;
}

bool DAIDepthInstance::hasNext() const
{
    if (m_file.is_open() && (m_frameIndex < m_nFrames || m_playLoop))
        return true;

    return false;
}

const DepthFrame& DAIDepthInstance::nextFrame()
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
        BinaryDepthFrame tempFrame[480];
        BinaryLabels tempLabel[480];
        m_file.read( (char *) tempFrame, sizeof(tempFrame) );
        m_file.read( (char *) tempLabel, sizeof(tempLabel) );

        for (int y=0; y<m_height; ++y) {
            for (int x=0; x<m_width; ++x)
            {
                //Loaded depths are already normalised because I did it when saved
                short int label = tempLabel[y].labelRow[x] > 0? 1 : 0;
                m_currentFrame.setItem(y, x, tempFrame[y].depthRow[x], label);
            }
        }

        m_frameIndex++;
    }
    else {
        close();
    }

    dilatateLabels();

    return m_currentFrame;
}

void DAIDepthInstance::dilatateLabels()
{
    cv::DataType<short int> dataType;
    cv::Mat newImag(480, 640, dataType.type, const_cast<short int*>(m_currentFrame.getLabelPtr()));
    int dilation_size = 15;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS,
                                               cv::Size(2*dilation_size + 1, 2*dilation_size+1),
                                               cv::Point( dilation_size, dilation_size ) );
    cv::dilate(newImag, newImag, kernel);
}

DepthFrame& DAIDepthInstance::frame()
{
    return m_currentFrame;
}


} // End namespace
