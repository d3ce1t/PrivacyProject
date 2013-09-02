#include "Scene3DPainter.h"

namespace dai {

Scene3DPainter::Scene3DPainter()
{
}

void Scene3DPainter::initialise()
{

}

void Scene3DPainter::render()
{
    // Configure ViewPort and Clear Screen
    glViewport(0, 0, m_width, m_height);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

} // End Namespace
