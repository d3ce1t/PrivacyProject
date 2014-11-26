#include "SceneItem.h"

namespace dai {

SceneItem::SceneItem(ItemType type)
    : m_scene(nullptr)
{
    m_type = type;
    m_z_order = 0;
    m_initialised = false;
    m_neededPasses = 1;
    m_visible = true;
    m_matrix.setToIdentity();
    m_matrix.ortho(0, 640, 480, 0, -1.0, 1.0);
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

void SceneItem::setMatrix(const QMatrix4x4& matrix)
{
    m_matrix = matrix; // Copy
}

int SceneItem::neededPasses() const
{
    return m_neededPasses;
}

ItemType SceneItem::type() const
{
    return m_type;
}

bool SceneItem::isVisible() const
{
    return m_visible;
}

void SceneItem::setVisible(bool value)
{
    m_visible = value;
}

void SceneItem::initItem()
{
    if (!m_initialised) {
        initializeOpenGLFunctions();
        initialise();
        m_initialised = true;
    }
}

void SceneItem::renderItem(int pass)
{
    if (!m_initialised) {
        initItem();
    }

    if (m_visible)
        render(pass);
}

} // End Namespace
