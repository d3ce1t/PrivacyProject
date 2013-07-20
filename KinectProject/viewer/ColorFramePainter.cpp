#include "ColorFramePainter.h"
#include "types/UserFrame.h"
#include "viewer/InstanceViewer.h"
#include <QImage>

namespace dai {

ColorFramePainter::ColorFramePainter(InstanceViewer *parent)
    : Painter(parent)
{
    m_frame = nullptr;
}

ColorFramePainter::~ColorFramePainter()
{
    m_frame = nullptr;
}

void ColorFramePainter::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();

    // Init Vertex Buffer
    prepareVertexBuffer();

    // Create FBO for Render-To-Texture
    createFrameBuffer();

    // Create texture
    glGenTextures(1, &m_foregroundTexture);
    glGenTextures(1, &m_maskTexture);
}

ColorFrame& ColorFramePainter::frame()
{
    return *m_frame;
}

void ColorFramePainter::prepareData(shared_ptr<DataFrame> frame)
{
    m_frame = static_pointer_cast<ColorFrame>(frame);
}

void ColorFramePainter::render()
{
    if (m_frame == nullptr)
        return;

    if (m_mask) {
        // Load Mask
        loadMaskTexture(m_maskTexture, m_mask->getWidth(), m_mask->getHeight(), (void *) m_mask->getDataPtr());
    }

    // Load Foreground
    loadVideoTexture(m_foregroundTexture, m_frame->getWidth(), m_frame->getHeight(), (void *) m_frame->getDataPtr());

    // Render to Texture
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboId);

    glViewport(0, 0, 640, 480);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //
    // Render scene to a texture directly
    //
    m_shaderProgram->bind();
    m_vao.bind();
    m_shaderProgram->setUniformValue(m_stageAttr, 1);
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);

    // Bind Foreground
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_foregroundTexture);

    // Bind Mask
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_maskTexture);

    // Bind Background Copy
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, m_backgroundTextureCopy);

    // Draw
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    // Copy frame buffer INTO m_backgroundTextureCopy
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 640, 480, 0);

    //
    // Render scene to Frame Buffer
    //
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, m_backgroundTexture);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Configure Viewport
    glViewport(0, 0, parent()->width(), parent()->height());

    // Render to Screen
    m_shaderProgram->setUniformValue(m_stageAttr, 2);

    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    // Unbind Background
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind Mask
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind Foreground
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_vao.release();
    m_shaderProgram->release();
}

void ColorFramePainter::createFrameBuffer()
{
    glGenTextures(1, &m_backgroundTexture);
    glBindTexture(GL_TEXTURE_2D, m_backgroundTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &m_backgroundTextureCopy);
    glBindTexture(GL_TEXTURE_2D, m_backgroundTextureCopy);
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
                           m_backgroundTexture,   // 4. tex ID
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

void ColorFramePainter::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/glsl/textureVertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/glsl/textureFragment.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("texCoord", 1);

    m_shaderProgram->link();

    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_texCoord = m_shaderProgram->attributeLocation("texCoord");
    m_stageAttr = m_shaderProgram->uniformLocation("stage");
    m_perspectiveMatrix = m_shaderProgram->uniformLocation("perspectiveMatrix");
    m_texColorSampler = m_shaderProgram->uniformLocation("texColor");
    m_texMaskSampler = m_shaderProgram->uniformLocation("texMask");
    m_texBackgroundSampler = m_shaderProgram->uniformLocation("texBackground");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_texColorSampler, 0);
    m_shaderProgram->setUniformValue(m_texMaskSampler, 1);
    m_shaderProgram->setUniformValue(m_texBackgroundSampler, 2);
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
    m_shaderProgram->release();
}

void ColorFramePainter::prepareVertexBuffer()
{
    float vertexData[] = {
        -1.0, -1.0, 0.0,
        1.0, -1.0, 0.0,
        1.0, 1.0, 0.0,
        -1.0, 1.0, 0.0
    };

    float texCoordsData[] = {
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

    m_texCoordBuffer.create();
    m_texCoordBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_texCoordBuffer.bind();
    m_texCoordBuffer.allocate(texCoordsData, 4*2*sizeof(float));
    m_shaderProgram->enableAttributeArray(m_texCoord);
    m_shaderProgram->setAttributeBuffer(m_texCoord, GL_FLOAT, 0, 2 );
    m_texCoordBuffer.release();

    m_vao.release();
}

// Create Texture (overwrite previous)
void ColorFramePainter::loadVideoTexture(GLuint glTextureId, GLsizei width, GLsizei height, void* texture)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Create Texture (overwrite previous)
void ColorFramePainter::loadMaskTexture(GLuint glTextureId, GLsizei width, GLsizei height, void* texture)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // End Namespace
