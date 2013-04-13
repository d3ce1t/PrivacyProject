#ifndef HISTOGRAMSCENE_H
#define HISTOGRAMSCENE_H

#include "viewer/ViewerPainter.h"
#include <QOpenGLBuffer>
#include <QMatrix4x4>

class QOpenGLShaderProgram;

namespace dai {

class HistogramScene
{
public:
    HistogramScene();
    virtual ~HistogramScene();
    void initialise();
    void update(float t);
    void render();
    void resize( float w, float h );
    void setMatrix(QMatrix4x4& m_matrix);
    void setHistogram(float histogram[]);

private:
    void prepareShaderProgram();

    QOpenGLShaderProgram*   m_shaderProgram;
    GLuint                  m_frameTexture; // Texture Object
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_posAttr; // Pos attr in the shader
    GLuint                  m_texCoord; // Texture coord in the shader
    GLuint                  m_texSampler; // Texture Sampler in the shader
    GLuint                  m_sampler; // Sampler
    QMatrix4x4              m_matrix;
    GLubyte                 m_histogram[100][100];
};

} // End Namespace

#endif // HISTOGRAMSCENE_H
