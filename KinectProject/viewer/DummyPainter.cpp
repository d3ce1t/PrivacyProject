#include "DummyPainter.h"
#include "InstanceViewer.h"

namespace dai {

DummyPainter::DummyPainter(InstanceViewer* parent)
    : Painter(parent), m_textureWidth(128), m_textureHeight(128)
{
    m_textureData = new uint8_t[m_textureWidth * m_textureHeight];
}

DummyPainter::~DummyPainter()
{
    delete[] m_textureData;
}

void DummyPainter::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();

    // Init Vertex Buffer
    prepareVertexBuffer();

    // Create texture
    createTexture(m_textureWidth, m_textureHeight, m_textureData);
    glGenTextures(1, &m_textureId);
    loadTexture(m_textureId, m_textureWidth, m_textureHeight, m_textureData);

    // Create FBO
    createFrameBuffer();
}

void DummyPainter::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/glsl/dummyVertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/glsl/dummyFragment.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("colorAttr", 1);
    m_shaderProgram->bindAttributeLocation("textureCoordAttr", 2);

    m_shaderProgram->link();

    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_colorAttr = m_shaderProgram->attributeLocation("colorAttr");
    m_textureCoordAttr = m_shaderProgram->attributeLocation("textureCoordAttr");
    m_textureSampler = m_shaderProgram->uniformLocation("textureSampler");
    m_posOffsetUniform = m_shaderProgram->uniformLocation("posOffset");
    m_useTextureUniform = m_shaderProgram->uniformLocation("useTexture");
    m_stageUniform = m_shaderProgram->uniformLocation("stage");

    // Initialise Uniformes
    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_textureSampler, 0);
    m_shaderProgram->setUniformValue(m_posOffsetUniform, 0, 0);
    m_shaderProgram->setUniformValue(m_useTextureUniform, false);
    m_shaderProgram->setUniformValue(m_stageUniform, 1);
    m_shaderProgram->release();
}

void DummyPainter::prepareVertexBuffer()
{

    float vertexData[] = {
        0.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 1.0, 0.0,
        0.0, 1.0, 0.0
    };

    float colorData[] = {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1,
        0, 0, 0
    };

    float textureCoordData[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };

    m_vao.create();
    m_vao.bind();

    m_positionsBuffer.create(); // Create a vertex buffer
    m_positionsBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_positionsBuffer.bind();
    m_positionsBuffer.allocate(vertexData, 4*3*sizeof(float));
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->setAttributeBuffer(m_posAttr, GL_FLOAT, 0, 3 );
    m_positionsBuffer.release();

    m_colorBuffer.create();
    m_colorBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_colorBuffer.bind();
    m_colorBuffer.allocate(colorData, 4*3*sizeof(float));
    m_shaderProgram->enableAttributeArray(m_colorAttr);
    m_shaderProgram->setAttributeBuffer(m_colorAttr, GL_FLOAT, 0, 3 );
    m_colorBuffer.release();

    m_textureCoordBuffer.create();
    m_textureCoordBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_textureCoordBuffer.bind();
    m_textureCoordBuffer.allocate(textureCoordData, 4*2*sizeof(float));
    m_shaderProgram->enableAttributeArray(m_textureCoordAttr);
    m_shaderProgram->setAttributeBuffer(m_textureCoordAttr, GL_FLOAT, 0, 2 );
    m_textureCoordBuffer.release();

    m_vao.release();
}

void DummyPainter::render()
{
    //
    // Render scene to a texture directly
    //
    enableRenderToTexture();

    m_shaderProgram->bind();
    m_vao.bind();


    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    m_shaderProgram->setUniformValue(m_stageUniform, 1);
    m_shaderProgram->setUniformValue(m_useTextureUniform, false);
    m_shaderProgram->setUniformValue(m_posOffsetUniform, -1, 0);
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    m_shaderProgram->setUniformValue(m_useTextureUniform, true);
    m_shaderProgram->setUniformValue(m_posOffsetUniform, 0, 0);
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    m_vao.release();
    m_shaderProgram->release();

    //
    // Render texture to Frame Buffer
    //
    renderToFrameBuffer();
}

void DummyPainter::prepareData(shared_ptr<DataFrame> frame)
{
    Q_UNUSED(frame)
}

void DummyPainter::loadTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DummyPainter::createTexture(int width, int height, uint8_t *data)
{    
    int bytesNumber = width * height * sizeof(uint8_t) / 2;
    memset(data, 255, bytesNumber); // First part of my texture is red, second part is black
}

void DummyPainter::createFrameBuffer()
{
    glGenTextures(1, &m_textureBG);
    glBindTexture(GL_TEXTURE_2D, m_textureBG);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // create a renderbuffer object to store depth info
    GLuint rboId;
    glGenRenderbuffers(1, &rboId);
    glBindRenderbuffer(GL_RENDERBUFFER, rboId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 640, 480);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // create a framebuffer object
    glGenFramebuffers(1, &m_fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    // attach the texture to FBO color attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER,        // 1. fbo target: GL_FRAMEBUFFER
                           GL_COLOR_ATTACHMENT0,  // 2. attachment point
                           GL_TEXTURE_2D,         // 3. tex target: GL_TEXTURE_2D
                           m_textureBG,           // 4. tex ID
                           0);                    // 5. mipmap level: 0(base)

    // attach the renderbuffer to depth attachment point
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,      // 1. fbo target: GL_FRAMEBUFFER
                              GL_DEPTH_ATTACHMENT, // 2. attachment point
                              GL_RENDERBUFFER,     // 3. rbo target: GL_RENDERBUFFER
                              rboId);              // 4. rbo ID

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if(status != GL_FRAMEBUFFER_COMPLETE)
        qDebug() << "FBO Error";

    // switch back to window-system-provided framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DummyPainter::enableRenderToTexture()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    glViewport(0, 0, 640, 480);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DummyPainter::renderToFrameBuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, m_textureBG);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Configure Viewport
    glViewport(0, 0, parent()->width(), parent()->height());

    m_shaderProgram->bind();
    m_vao.bind();
    m_shaderProgram->setUniformValue(m_stageUniform, 2);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_textureBG);

    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_vao.release();
    m_shaderProgram->release();
}


} // End Namespace
