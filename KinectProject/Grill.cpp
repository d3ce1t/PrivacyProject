#include "Grill.h"
#include <QOpenGLShaderProgram>

Grill::Grill()
{
    m_shaderProgram = NULL;
}

Grill::~Grill()
{
    if (m_shaderProgram != NULL)
    {
        delete m_shaderProgram;
        m_shaderProgram = NULL;
    }
}

void Grill::initialise()
{
    prepareShaderProgram();
}

void Grill::update(float t)
{

}

void Grill::render()
{
    float vertexData[] = {
        -1.0, 0.0, -1.0,
        1.0, 0.0, -1.0,
        1.0, 0.0, 1.0,
        -1.0, 0.0, 1.0
    };

    float colorData[] = {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        0.0, 0.0, 0.0
    };

    // Render
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    //glEnableClientState(GL_VERTEX_ARRAY);
    //glEnableClientState(GL_COLOR_ARRAY);

    m_shaderProgram->bind();
    m_shaderProgram->setAttributeArray(m_posAttr, vertexData, 3);
    m_shaderProgram->setAttributeArray(m_colorAttr, colorData, 3);
    m_shaderProgram->setUniformValue(m_pointSize, 3.0f);
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_colorAttr);

    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    m_shaderProgram->disableAttributeArray(m_colorAttr);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    m_shaderProgram->release();

    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
    //glDisableClientState(GL_VERTEX_ARRAY);
    //glDisableClientState(GL_COLOR_ARRAY);
}

void Grill::resize( float w, float h )
{

}

void Grill::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/simpleVertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/simpleFragment.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("colAttr", 1);

    m_shaderProgram->link();

    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_colorAttr = m_shaderProgram->attributeLocation("colAttr");
    m_pointSize = m_shaderProgram->uniformLocation("sizeAttr");
    m_perspectiveMatrix = m_shaderProgram->uniformLocation("perspectiveMatrix");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
    m_shaderProgram->setUniformValue(m_pointSize, 4.5f);
    m_shaderProgram->release();
}

void Grill::setMatrix(QMatrix4x4& matrix) {
    this->m_matrix = matrix;
}
