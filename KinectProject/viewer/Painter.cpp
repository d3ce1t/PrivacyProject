#include "Painter.h"

namespace dai {

Painter::Painter(QOpenGLContext* context)
{
    m_initialised = false;
    m_context = context;
    m_shaderProgram = NULL;
}

Painter::~Painter()
{
    if (m_shaderProgram != NULL) {
        delete m_shaderProgram;
        m_shaderProgram = NULL;
    }

    m_initialised = false;
    m_context = NULL;
}

void Painter::setMatrix(QMatrix4x4& matrix) {
    this->m_matrix = matrix;
}

void Painter::renderNow()
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
