#include "ViewerPainter.h"

namespace dai {

ViewerPainter::ViewerPainter(DataInstance *instance)
{
    m_initialised = false;
    m_instance = instance;
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

DataInstance &ViewerPainter::instance() const
{
    return *m_instance;
}

} // End Namespace
