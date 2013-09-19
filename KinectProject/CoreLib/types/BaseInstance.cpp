#include "BaseInstance.h"

namespace dai {

BaseInstance::BaseInstance()
{
}

auto BaseInstance::getType() const -> InstanceType
{
    return m_type;
}

const QString& BaseInstance::getTitle() const
{
    return m_title;
}

} // End Namespace
