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
    m_items.insert(item->getZOrder(), item);
}

void ScenePainter::setBackground(shared_ptr<DataFrame> background)
{
    m_bg = background;
    m_needLoading.store(1);
}

void ScenePainter::setSize(qreal width, qreal height)
{
    m_width = width;
    m_height = height;
}

void ScenePainter::setMatrix(const QMatrix4x4 &matrix)
{
    m_matrix = matrix;
    updateItemsMatrix();
}

QMatrix4x4& ScenePainter::getMatrix()
{
    return m_matrix;
}

void ScenePainter::updateItemsMatrix()
{
    /*m_mutex.lock();
    foreach (dai::Painter* painter, m_painters) {
        painter->setMatrix(m_matrix);
    }
    m_mutex.unlock();

    if (m_window != nullptr)
        m_window->update();*/
}

void ScenePainter::resetPerspective()
{
    m_matrix.setToIdentity();
    m_matrix.perspective(43, 640/480, 0.01, 50.0);
    updateItemsMatrix();
}

void ScenePainter::renderScene()
{
    if (!m_initialised)
    {
        initializeOpenGLFunctions();
        initialise();
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

} // End Namespace
