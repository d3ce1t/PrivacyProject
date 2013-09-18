#include "SceneItem.h"

namespace dai {

SceneItem::SceneItem(ItemType type)
{
    m_type = type;
    m_z_order = 0;
    m_initialised = false;
    m_neededPasses = 1;
}

int SceneItem::getZOrder() const
{
    return m_z_order;
}

ScenePainter* SceneItem::scene() const
{
    return m_scene;
}

void SceneItem::setBackgroundTex(GLuint id)
{
    m_fgTextureId = id;
}

int SceneItem::neededPasses() const
{
    return m_neededPasses;
}

ItemType SceneItem::type() const
{
    return m_type;
}

void SceneItem::renderItem(int pass)
{
    if (!m_initialised) {
        initializeOpenGLFunctions();
        initialise();
        m_initialised = true;
    }

    render(pass);
}

} // End Namespace
