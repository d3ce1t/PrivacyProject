#include "simpleshaderprogram.h"

SimpleShaderProgram::SimpleShaderProgram()
{
    this->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/simpleVertex.vsh");
    this->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/simpleFragment.fsh");
    this->bindAttributeLocation("posAttr", 0);
    this->bindAttributeLocation("colAttr", 1);

    this->link();

    m_posAttr = this->attributeLocation("posAttr");
    m_colorAttr = this->attributeLocation("colAttr");
    m_perspectiveMatrix = this->uniformLocation("perspectiveMatrix");
    m_pointSize = this->uniformLocation("size");
}

void SimpleShaderProgram::setPosAttribute(const GLfloat *values, int tupleSize) {
    this->setAttributeArray(m_posAttr, values, tupleSize);
}

void SimpleShaderProgram::setColorAttribute(const GLfloat *values, int tupleSize) {
    this->setAttributeArray(m_colorAttr, values, tupleSize);
}

void SimpleShaderProgram::setPointSize(const GLfloat size) {
    this->setUniformValue(m_pointSize, size);
}

void SimpleShaderProgram::setMatrix(QMatrix4x4 matrix) {
    this->bind();
    this->setUniformValue(m_perspectiveMatrix, matrix);

}

void SimpleShaderProgram::enableAttributes() {
    this->enableAttributeArray(m_posAttr);
    this->enableAttributeArray(m_colorAttr);
}

void SimpleShaderProgram::disableAttributes() {
    this->disableAttributeArray(m_colorAttr);
    this->disableAttributeArray(m_posAttr);
}

bool SimpleShaderProgram::bind() {
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    return QOpenGLShaderProgram::bind();
}

void SimpleShaderProgram::release() {
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
    return QOpenGLShaderProgram::release();
}
