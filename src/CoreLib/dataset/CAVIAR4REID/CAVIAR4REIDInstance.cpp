#include "CAVIAR4REIDInstance.h"
#include "dataset/DatasetMetadata.h"
#include <QFile>
#include "Utils.h"
#include <QDebug>

namespace dai {

CAVIAR4REIDInstance::CAVIAR4REIDInstance(const InstanceInfo &info)
    : DataInstance(info, DataFrame::Color, -1, -1)
{
    m_colorFrame = make_shared<ColorFrame>();
    m_width = 0;
    m_height = 0;
}

CAVIAR4REIDInstance::~CAVIAR4REIDInstance()
{
    m_width = 0;
    m_height = 0;
    closeInstance();
}

bool CAVIAR4REIDInstance::is_open() const
{
    return m_frameBuffer.data != nullptr;
}

bool CAVIAR4REIDInstance::hasNext() const
{
    return true;
}

bool CAVIAR4REIDInstance::openInstance()
{
    bool result = false;
    QString instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Color);
    QFile file(instancePath);

    if (file.exists()) {
        result = true;
    }

    return result;
}

void CAVIAR4REIDInstance::closeInstance()
{   
}

void CAVIAR4REIDInstance::restartInstance()
{
}

void CAVIAR4REIDInstance::nextFrame(QHashDataFrames &output)
{
    QString instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Color);
    m_frameBuffer = cv::imread(instancePath.toStdString());

    if (m_frameBuffer.data) {
        cv::cvtColor(m_frameBuffer, m_frameBuffer, CV_BGR2RGB);

        // Scale
        if (m_frameBuffer.cols < 64) {
            double scale_factor = 64.0 / m_frameBuffer.cols;
            cv::Size size(64, scale_factor*m_frameBuffer.rows);
            cv::resize(m_frameBuffer, m_frameBuffer, size);//resize image
        }

        m_colorFrame->setDataPtr(m_frameBuffer.cols, m_frameBuffer.rows, (RGBColor*) m_frameBuffer.data);
    }

    if (output.size() == 0) {
        output.insert(DataFrame::Color, m_colorFrame);
    } else {
        shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(output.value(DataFrame::Color));
        *colorFrame = *m_colorFrame; // Copy
    }
}

} // End namespace
