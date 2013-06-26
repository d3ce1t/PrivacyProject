#include "StreamInstance.h"
#include <QDebug>

namespace dai {

StreamInstance::StreamInstance()
{
    m_type = StreamInstance::Uninitialised;
}

StreamInstance::~StreamInstance()
{
    m_type = StreamInstance::Uninitialised;
    qDebug() << "StreamInstance::~StreamInstance()";
}

StreamInstance::StreamType StreamInstance::getType() const
{
    return m_type;
}

const QString& StreamInstance::getTitle() const
{
    return m_title;
}

} // End namespace
