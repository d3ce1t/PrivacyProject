#ifndef SIMPLESHADERPROGRAM_H
#define SIMPLESHADERPROGRAM_H

#include <QOpenGLShaderProgram>

class SimpleShaderProgram : public QOpenGLShaderProgram
{
public:
    explicit SimpleShaderProgram();
    void setPosAttribute(const GLfloat *values, int tupleSize);
    void setColorAttribute(const GLfloat *values, int tupleSize);
    void setPointSize(const GLfloat size);
    void setMatrix(QMatrix4x4 matrix);
    void enableAttributes();
    void disableAttributes();
    GLuint getPosAttribute() {return m_posAttr;}
    GLuint getColorAttribute() {return m_colorAttr;}
    bool bind();
    void release();

private:
    GLuint                  m_posAttr;
    GLuint                  m_colorAttr; // Texture coord in the shader
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_pointSize;
};

#endif // SIMPLESHADERPROGRAM_H
