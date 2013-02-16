#include "trianglewindow.h"
#include <QtGui/QMatrix4x4>
#include <QtGui/QScreen>
#include <QtCore/qmath.h>
#include <iostream>

using namespace std;


GLfloat vertices[] = {
    0.0f, 0.707f,
    -0.5f, -0.5f,
    0.5f, -0.5f
};

const GLfloat colours[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};


TriangleWindow::TriangleWindow()
    : m_program(0) {}


void TriangleWindow::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex/vertex.vsh");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragment/fragment.fsh");
    m_program->link();
    m_colAttr = m_program->attributeLocation("colAttr");
    m_posAttr = m_program->attributeLocation("posAttr");
}

void TriangleWindow::render()
{
    glViewport(0, 0, width(), height());

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    m_program->setAttributeArray(m_posAttr, vertices, 2);
    m_program->setAttributeArray(m_colAttr, colours, 3);

    m_program->enableAttributeArray(m_posAttr);
    m_program->enableAttributeArray(m_colAttr);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    m_program->disableAttributeArray(m_colAttr);
    m_program->disableAttributeArray(m_posAttr);

    m_program->release();
}
