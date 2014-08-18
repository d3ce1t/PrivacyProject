#include "CAVIAR4REIDInstance.h"
#include "dataset/DatasetMetadata.h"
#include <QFile>

namespace dai {

CAVIAR4REIDInstance::CAVIAR4REIDInstance(const InstanceInfo &info)
    : DataInstance(info)
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
    return !m_frameBuffer.isNull();
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

    if (file.exists())
    {
        if (!m_frameBuffer.load(instancePath))
            return result;

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

QList<shared_ptr<DataFrame> > CAVIAR4REIDInstance::nextFrame()
{
    QList<shared_ptr<DataFrame>> result;
    m_colorFrame->setDataPtr(m_frameBuffer.width(), m_frameBuffer.height(), (RGBColor*) m_frameBuffer.constBits());
    result.append(m_colorFrame);
    return result;
}

} // End namespace
