#include "Painter.h"
#include "types/UserFrame.h"

namespace dai {

Painter::Painter(QOpenGLContext* context)
{
    m_initialised = false;
    m_context = context;
    m_shaderProgram = nullptr;
}

Painter::~Painter()
{
    if (m_shaderProgram != nullptr) {
        delete m_shaderProgram;
        m_shaderProgram = nullptr;
    }

    m_initialised = false;
    m_context = nullptr;
}

void Painter::setMatrix(QMatrix4x4& matrix) {
    this->m_matrix = matrix;
}

void Painter::setMask(shared_ptr<UserFrame> mask)
{
    m_mask = mask;
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
