#ifndef GRILL_H
#define GRILL_H

#include "AbstractScene.h"
#include <QOpenGLBuffer>
#include <QMatrix4x4>

class QOpenGLShaderProgram;

class Grill : public AbstractScene
{
public:
    Grill();
    virtual ~Grill();
    void initialise();
    void update(float t);
    void render();
    void resize( float w, float h );
    void setMatrix(QMatrix4x4& m_matrix);

private:
    void prepareShaderProgram();

    QOpenGLShaderProgram*   m_shaderProgram;
    GLuint                  m_posAttr;
    GLuint                  m_colorAttr; // Texture coord in the shader
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_pointSize;
    QMatrix4x4              m_matrix;
};

#endif // GRILL_H
