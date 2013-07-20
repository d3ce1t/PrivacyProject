#include "Painter.h"
#include "types/UserFrame.h"

namespace dai {

Painter::Painter(InstanceViewer *parent)
{
    m_initialised = false;
    m_viewer = parent;
    m_shaderProgram = nullptr;
}

Painter::~Painter()
{
    if (m_shaderProgram != nullptr) {
        delete m_shaderProgram;
        m_shaderProgram = nullptr;
    }

    m_initialised = false;
    m_viewer = nullptr;
}

InstanceViewer* Painter::parent() const
{
    return m_viewer;
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
