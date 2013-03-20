#include "basicusagescene.h"

#include <QOpenGLShaderProgram>

BasicUsageScene::BasicUsageScene()
    : m_shaderProgram(NULL),
      m_vertexPositionBuffer( QOpenGLBuffer::VertexBuffer),
      m_vertexColorBuffer( QOpenGLBuffer::VertexBuffer)
{
    currentTime = 0;
}

BasicUsageScene::~BasicUsageScene()
{
    if (m_shaderProgram != NULL)
    {
        delete m_shaderProgram;
        m_shaderProgram = NULL;
    }
}

void BasicUsageScene::initialise()
{
    // Create VAO
    //m_vao.create() // Not supported until 5.1
    //m_vao.bind();

    // Load, compile and link the shader program
    prepareShaderProgram();

    // Prepare our geometry and associate it with shader program
    //prepareVertexBuffers();

    //m_vao.release();

    // Set the clear color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void BasicUsageScene::update( float t )
{
    currentTime = t;
}

void BasicUsageScene::render()
{
    const GLfloat vertexData[] = {
        0.0f, 0.707f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };

    const GLfloat colorData[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaderProgram->bind();

    m_shaderProgram->setUniformValue(m_time, currentTime);

    //m_vao.bind();

    // Because VAO isnt' supported, do this

    m_shaderProgram->setAttributeArray(m_posAttr, vertexData, 2);
    m_shaderProgram->setAttributeArray(m_colAttr, colorData, 3);

    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_colAttr);
    // end

    // Draw
    glDrawArrays(GL_TRIANGLES, m_posAttr, 3);

    m_shaderProgram->setUniformValue(m_time, currentTime + 2.5f);
    glDrawArrays(GL_TRIANGLES, m_posAttr, 3);

    // Because VAO isnt' supported, do this
    m_shaderProgram->disableAttributeArray(m_colAttr);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    // end

    //m_vao.release();

    m_shaderProgram->release();
}

void BasicUsageScene::resize( int w, int h )
{
    width = w;
    height = h;
}

void BasicUsageScene::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/sampleVertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/sampleFragment.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("colAttr", 1);

    m_shaderProgram->link();

    m_colAttr = m_shaderProgram->attributeLocation("colAttr");
    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_loopDuration = m_shaderProgram->uniformLocation("loopDuration");
    m_fragLoopDuration = m_shaderProgram->uniformLocation("fragLoopDuration");
    m_time = m_shaderProgram->uniformLocation("time");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_loopDuration, 5.0f);
    m_shaderProgram->setUniformValue(m_fragLoopDuration, 10.0f);
    m_shaderProgram->release();
}


void BasicUsageScene::prepareVertexBuffers()
{
    const GLfloat vertexData[] = {
        0.0f, 0.707f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };

    const GLfloat colorData[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    m_shaderProgram->bind();

    m_vertexPositionBuffer.create();
    m_vertexPositionBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_vertexPositionBuffer.bind();
    m_vertexPositionBuffer.allocate( vertexData, 3 * 2 * sizeof(GLfloat) );
    m_shaderProgram->setAttributeBuffer(m_posAttr, GL_FLOAT, 0, 2, 0);
    m_vertexPositionBuffer.release();

    m_vertexColorBuffer.create();
    m_vertexColorBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_vertexColorBuffer.bind();
    m_vertexColorBuffer.allocate( colorData, 3 * 3 * sizeof(GLfloat) );
    m_shaderProgram->setAttributeBuffer(m_colAttr, GL_FLOAT, 0, 3, 0);
    m_vertexColorBuffer.release();

    m_shaderProgram->release();
}
