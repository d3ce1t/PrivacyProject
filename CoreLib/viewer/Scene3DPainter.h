#ifndef SCENE3DPAINTER_H
#define SCENE3DPAINTER_H

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include "viewer/ScenePainter.h"

namespace dai {

class Scene3DPainter : public ScenePainter
{
public:
    Scene3DPainter();

protected:
    void initialise() override;
    void render() override;

private:
    void prepareShaderProgram();
    void prepareVertexBuffer();

    QOpenGLShaderProgram*   m_shaderProgram;

    // OpenGL Buffer
    QOpenGLVertexArrayObject   m_vao;
    QOpenGLBuffer              m_positionsBuffer;
    QOpenGLBuffer              m_distancesBuffer;

    // OpenGL identifiers
    GLuint                  m_perspectiveMatrix; // Matrix in the shader
    GLuint                  m_indexAttr; // Pos attr in the shader
    GLuint                  m_distanceAttr;
    GLuint                  m_widthUniform;
    GLuint                  m_heightUniform;


};

} // End Namespace

#endif // SCENE3DPAINTER_H
