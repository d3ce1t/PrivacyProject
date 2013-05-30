#include "trianglewindow.h"
#include <QtGui/QMatrix4x4>
#include <QtGui/QScreen>
#include <QtCore/qmath.h>
#include <iostream>
#include <QElapsedTimer>
#include <cmath>

using namespace std;


GLfloat vertices[] = {
    // Triangle 1
    0.25f,  0.25f, -1.25f, 1.0f,
    0.25f, -0.25f, -1.25f, 1.0f,
   -0.25f,  0.25f, -1.25f, 1.0f,

    // Triangle 2
    0.25f, -0.25f, -1.25f, 1.0f,
   -0.25f, -0.25f, -1.25f, 1.0f,
   -0.25f,  0.25f, -1.25f, 1.0f,

    // Triangle 3
    0.25f,  0.25f, -2.75f, 1.0f,
   -0.25f,  0.25f, -2.75f, 1.0f,
    0.25f, -0.25f, -2.75f, 1.0f,

    // Triangle 4
    0.25f, -0.25f, -2.75f, 1.0f,
   -0.25f,  0.25f, -2.75f, 1.0f,
   -0.25f, -0.25f, -2.75f, 1.0f,

    // Triangle 5
   -0.25f,  0.25f, -1.25f, 1.0f,
   -0.25f, -0.25f, -1.25f, 1.0f,
   -0.25f, -0.25f, -2.75f, 1.0f,

    // Triangle 6
   -0.25f,  0.25f, -1.25f, 1.0f,
   -0.25f, -0.25f, -2.75f, 1.0f,
   -0.25f,  0.25f, -2.75f, 1.0f,

    // Triangle 7
    0.25f,  0.25f, -1.25f, 1.0f,
    0.25f, -0.25f, -2.75f, 1.0f,
    0.25f, -0.25f, -1.25f, 1.0f,

    // Triangle 8
    0.25f,  0.25f, -1.25f, 1.0f,
    0.25f,  0.25f, -2.75f, 1.0f,
    0.25f, -0.25f, -2.75f, 1.0f,

    // Triangle 9
    0.25f,  0.25f, -2.75f, 1.0f,
    0.25f,  0.25f, -1.25f, 1.0f,
   -0.25f,  0.25f, -1.25f, 1.0f,

    // Triangle 10
    0.25f,  0.25f, -2.75f, 1.0f,
   -0.25f,  0.25f, -1.25f, 1.0f,
   -0.25f,  0.25f, -2.75f, 1.0f,

    // Triangle 11
    0.25f, -0.25f, -2.75f, 1.0f,
   -0.25f, -0.25f, -1.25f, 1.0f,
    0.25f, -0.25f, -1.25f, 1.0f,

    // Triangle 12
    0.25f, -0.25f, -2.75f, 1.0f,
   -0.25f, -0.25f, -2.75f, 1.0f,
   -0.25f, -0.25f, -1.25f, 1.0f
};

const GLfloat colours[] = {
    // Color for Triangle 1
    0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,

    // Color for Triangle 2
    0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f,

    // Color for Triangle 3
    0.8f, 0.8f, 0.8f, 1.0f,
    0.8f, 0.8f, 0.8f, 1.0f,
    0.8f, 0.8f, 0.8f, 1.0f,

    // Color for Triangle 4
    0.8f, 0.8f, 0.8f, 1.0f,
    0.8f, 0.8f, 0.8f, 1.0f,
    0.8f, 0.8f, 0.8f, 1.0f,

    // Color for Triangle 5
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,

    // Color for Triangle 6
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,

    // Color for Triangle 7
    0.5f, 0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.0f, 1.0f,

    // Color for Triangle 8
    0.5f, 0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.0f, 1.0f,

    // Color for Triangle 9
    1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,

    // Color for Triangle 10
    1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f,

    // Color for Triangle 11
    0.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f, 1.0f,

    // Color for Triangle 12
    0.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f, 1.0f
};

TriangleWindow::TriangleWindow()
    : m_program(0)
{
    fFrustumScale = 1.0f;
    fzNear = 0.3f;
    fzFar = 100.0f;
}


void TriangleWindow::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex/vertex.vsh");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragment/fragment.fsh");
    m_program->bindAttributeLocation("posAttr", 0);
    m_program->bindAttributeLocation("colAttr", 1);

    m_program->link();

    m_colAttr = m_program->attributeLocation("colAttr");
    m_posAttr = m_program->attributeLocation("posAttr");
    m_offset = m_program->uniformLocation("offset");
    m_perspectiveMatrix = m_program->uniformLocation("perspectiveMatrix");

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    matrix.fill(0);
    matrix(0, 0) = fFrustumScale;
    matrix(1,1) = fFrustumScale;
    matrix(2,2) = (fzFar + fzNear) / (fzNear - fzFar);
    matrix(3, 2) = (2 * fzFar * fzNear) / (fzNear - fzFar);
    matrix(2, 3) = -1.0f;

    /*QMatrix4x4 matrix;
    matrix.perspective(60, 1.0, 0.1, 10.0);
    matrix.translate(0, 0, -2);
    matrix.rotate(10, 0, 1, 0);*/

    //cout << screen()->refreshRate() << endl;

    m_program->bind();
    m_program->setUniformValue(m_perspectiveMatrix, matrix);
    m_program->release();

    timer.start();
}

void TriangleWindow::render()
{
    glViewport(0, 0, width(), height());
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    m_program->setUniformValue(m_offset, 0.5, 0.5);
    m_program->setAttributeArray(m_posAttr, vertices, 4);
    m_program->setAttributeArray(m_colAttr, colours, 4);

    m_program->enableAttributeArray(m_posAttr);
    m_program->enableAttributeArray(m_colAttr);

    glDrawArrays(GL_TRIANGLES, m_posAttr, 36);

    m_program->disableAttributeArray(m_colAttr);
    m_program->disableAttributeArray(m_posAttr);

    m_program->release();
}

void TriangleWindow::resizeEvent(QResizeEvent *event)
{
    /*matrix(0, 0) = fFrustumScale / (width() / (float) height());
    matrix(1,1) = fFrustumScale;

    if (m_program) {
        m_program->bind();
        m_program->setUniformValue(m_perspectiveMatrix, matrix);
        m_program->release();
    }*/

    OpenGLWindow::resizeEvent(event);
}
