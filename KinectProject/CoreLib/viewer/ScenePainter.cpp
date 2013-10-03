#include "ScenePainter.h"

namespace dai {

ScenePainter::ScenePainter()
{
    m_initialised = false;
    m_needLoading.store(0);
}

ScenePainter::~ScenePainter()
{

}

void ScenePainter::clearItems()
{
    m_items.clear();
}

void ScenePainter::addItem(shared_ptr<SceneItem> item)
{
    item->m_scene = this;
    m_items.append(item);
}

shared_ptr<SceneItem> ScenePainter::getFirstItem(ItemType type) const
{
    shared_ptr<SceneItem> result;
    QListIterator<shared_ptr<SceneItem>> it(m_items);

    while (!result && it.hasNext()) {

        shared_ptr<SceneItem> item = it.next();

        if (item->type() == type) {
            result = item;
        }
    }

    return result;
}

void ScenePainter::setBackground(shared_ptr<DataFrame> background)
{
    m_bg = background;
    m_needLoading.store(1);
}

void ScenePainter::setWindowSize(int width, int height)
{
    m_window_width = width;
    m_window_height = height;
}

int ScenePainter::windowWidth() const
{
    return m_window_width;
}

int ScenePainter::windowHeight() const
{
    return m_window_height;
}

void ScenePainter::setMatrix(const QMatrix4x4 &matrix)
{
    m_matrix = matrix;
}

QMatrix4x4& ScenePainter::getMatrix()
{
    return m_matrix;
}

void ScenePainter::resetPerspective()
{
    m_matrix.setToIdentity();
    m_matrix.perspective(45, 4/3, 0.1f, 100.0f);
}

void ScenePainter::renderScene()
{
    if (!m_initialised)
    {
        initializeOpenGLFunctions();
        initialise();
        resetPerspective();
        m_initialised = true;
    }

    render();
}

void ScenePainter::renderItems()
{
    foreach (shared_ptr<SceneItem> item, m_items)
    {
        item->renderItem();
    }
}

//
// Static Methods
//

// Create Texture (overwrite previous)
void ScenePainter::loadVideoTexture(GLuint glTextureId, GLsizei width, GLsizei height, void* texture)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Create Texture (overwrite previous)
void ScenePainter::loadMaskTexture(GLuint glTextureId, GLsizei width, GLsizei height, void* texture)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}


} // End Namespace