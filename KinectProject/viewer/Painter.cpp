#include "Painter.h"

namespace dai {

Painter::Painter(StreamInstance *instance, InstanceViewer* parent)
{
    m_initialised = false;
    m_instance = instance;
    m_viewer = parent;
}

Painter::~Painter()
{
    m_initialised = false;
    m_viewer = NULL;
}

void Painter::setMatrix(QMatrix4x4& matrix) {
    this->m_matrix = matrix;
}

void Painter::renderNow()
{
    if (!m_initialised)
    {
        initialise();
        m_initialised = true;
    }

    render();
}

} // End Namespace
