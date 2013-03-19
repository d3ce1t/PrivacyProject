#ifndef BASICUSAGESCENE_H
#define BASICUSAGESCENE_H

#include <QOpenGLBuffer>

class QOpenGLShaderProgram;

class BasicUsageScene
{
public:
    explicit BasicUsageScene();
    virtual ~BasicUsageScene();
    virtual void initialise();
    virtual void update(float t);
    virtual void render();
    virtual void resize( int w, int h );

private:
    void prepareShaderProgram();
    void prepareVertexBuffers();

    QOpenGLShaderProgram* m_shaderProgram;
    QOpenGLBuffer m_vertexPositionBuffer;
    QOpenGLBuffer m_vertexColorBuffer;
    //QOpenGLVertexArrayObject m_vao;

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_loopDuration;
    GLuint m_fragLoopDuration;
    GLuint m_time;

    float currentTime;
    float width;
    float height;
};

#endif // BASICUSAGESCENE_H
