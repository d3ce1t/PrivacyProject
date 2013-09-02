#include "SceneItem.h"

namespace dai {

SceneItem::SceneItem()
{
    m_z_order = 0;
}

int SceneItem::getZOrder() const
{
    return m_z_order;
}

} // End Namespace
