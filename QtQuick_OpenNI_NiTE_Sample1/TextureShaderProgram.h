#ifndef TEXTURE_SHADER_PROGRAM_H
#define TEXTURE_SHADER_PROGRAM_H

#include <QtGui/QOpenGLShaderProgram>

class TextureShaderProgram : protected QOpenGLShaderProgram
{
public:
    explicit TextureShaderProgram();
    void setPosAttribute(const GLfloat *values, int tupleSize);
    void setTexCoordAttribute(const GLfloat *values, int tupleSize);
    void setMatrix(QMatrix4x4 matrix);
    void enableAttributes();
    void disableAttributes();
    GLuint getPosAttribute() {return m_posAttr;}
    GLuint getTexCoordAttribute() {return m_texCoord;}
    bool bind();
    void release();

private:
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_posAttr; // Pos attr in the shader
    GLuint                  m_texCoord; // Texture coord in the shader
    GLuint                  m_texSampler; // Texture Sampler in the shader
    GLuint                  m_sampler; // Sampler
};

#endif // TEXTURE_SHADER_PROGRAM_H
