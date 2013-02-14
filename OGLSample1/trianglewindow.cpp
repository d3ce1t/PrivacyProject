#include "trianglewindow.h"
#include <QtGui/QMatrix4x4>
#include <QtGui/QScreen>
#include <QtCore/qmath.h>
#include <iostream>

using namespace std;


const GLfloat vertices[] = {
    0.0f, 0.707f,
    -0.5f, -0.5f,
    0.5f, -0.5f
};

static const char *vertexShaderSource =
        "#version 130\n"
        "in vec4 posAttr;\n"
        "void main() {\n"
        "   gl_Position = posAttr;\n"
        "}\n";

static const char *fragmentShaderSource =
        "#version 130\n"
        "out vec4 outputColor;\n"
        "void main() {\n"
        "   outputColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "}\n";

TriangleWindow::TriangleWindow()
    : m_program(0) {}

GLuint TriangleWindow::loadShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;
}

void TriangleWindow::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = m_program->attributeLocation("posAttr");
}

void TriangleWindow::render()
{
    glViewport(0, 0, width(), height());

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    m_program->enableAttributeArray(m_posAttr);
    //glEnableVertexAttribArray(m_posAttr);

    m_program->setAttributeArray(m_posAttr, vertices, 2);
    //glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    glDrawArrays(GL_TRIANGLES, m_posAttr, 3);

    m_program->disableAttributeArray(m_posAttr);
    //glDisableVertexAttribArray(m_posAttr);

    m_program->release();
}
