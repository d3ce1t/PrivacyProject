#include "SceneItem.h"

namespace dai {

SceneItem::SceneItem()
{
    m_z_order = 0;
    m_initialised = false;
}

int SceneItem::getZOrder() const
{
    return m_z_order;
}

ScenePainter* SceneItem::scene() const
{
    return m_scene;
}

void SceneItem::renderItem()
{
    if (!m_initialised)
    {
        initializeOpenGLFunctions();
        initialise();
        m_initialised = true;
    }

    render();
}

} // End Namespace
