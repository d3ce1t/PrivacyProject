#include "ViewerPainter.h"

namespace dai {

ViewerPainter::ViewerPainter(StreamInstance *instance, InstanceViewer* parent)
{
    m_initialised = false;
    m_instance = instance;
    m_viewer = parent;
}

ViewerPainter::~ViewerPainter()
{
    m_initialised = false;
    m_instance = NULL;
    m_viewer = NULL;
}

void ViewerPainter::setMatrix(QMatrix4x4& matrix) {
    this->m_matrix = matrix;
}

void ViewerPainter::renderNow()
{
    if (!m_initialised)
    {
        initialise();
        m_initialised = true;
    }

    render();
}

StreamInstance &ViewerPainter::instance() const
{
    return *m_instance;
}

} // End Namespace
