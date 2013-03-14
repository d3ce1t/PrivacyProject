#include "TextureShaderProgram.h"

TextureShaderProgram::TextureShaderProgram()
{
    this->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/textureVertex.vsh");
    this->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/textureFragment.fsh");
    this->bindAttributeLocation("posAttr", 0);
    this->bindAttributeLocation("texCoord", 1);

    this->link();

    m_posAttr = this->attributeLocation("posAttr");
    m_texCoord = this->attributeLocation("texCoord");
    m_perspectiveMatrix = this->uniformLocation("perspectiveMatrix");
    m_texSampler = this->uniformLocation("texSampler");

    this->bind();
    this->setUniformValue(m_texSampler, 0);
    this->release();
}

void TextureShaderProgram::setPosAttribute(const GLfloat *values, int tupleSize) {
    this->setAttributeArray(m_posAttr, values, tupleSize);
}

void TextureShaderProgram::setTexCoordAttribute(const GLfloat *values, int tupleSize) {
    this->setAttributeArray(m_texCoord, values, tupleSize);
}

void TextureShaderProgram::setMatrix(QMatrix4x4 matrix) {
    this->bind();
    this->setUniformValue(m_perspectiveMatrix, matrix);
    this->release();
}

void TextureShaderProgram::enableAttributes() {
    this->enableAttributeArray(m_posAttr);
    this->enableAttributeArray(m_texCoord);
}

void TextureShaderProgram::disableAttributes() {
    this->disableAttributeArray(m_texCoord);
    this->disableAttributeArray(m_posAttr);
}

bool TextureShaderProgram::bind() {
    return QOpenGLShaderProgram::bind();
}

void TextureShaderProgram::release() {
    return QOpenGLShaderProgram::release();
}
