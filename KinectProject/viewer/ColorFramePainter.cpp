#include "ColorFramePainter.h"
#include "types/UserFrame.h"
#include "viewer/InstanceViewer.h"
#include <QQuickWindow>
#include <QImage>

namespace dai {

ColorFramePainter::ColorFramePainter(InstanceViewer *parent)
    : Painter(parent)
{
    m_frame = nullptr;
    m_needLoading.store(0);
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
    glGenTextures(1, &m_fgTextureId);
    glGenTextures(1, &m_maskTextureId);
}

void ColorFramePainter::render()
{
    if (m_frame == nullptr)
        return;

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_perspectiveMatrixUniform, m_matrix);
    m_shaderProgram->setUniformValue(m_currentFilterUniform, m_currentFilter);

    enableBGRendering();
    renderBackground();
    m_fbo->release();

    enableFilterRendering();
    renderFilter();
    m_fboFilter->release();

    displayRenderedTexture();

    m_shaderProgram->release();
    glDisable(GL_TEXTURE_2D);
}

void ColorFramePainter::renderBackground()
{
    m_vao.bind();

    if (m_needLoading.load())
    {
        // Load Foreground
        loadVideoTexture(m_fgTextureId, m_frame->getWidth(), m_frame->getHeight(), (void *) m_frame->getDataPtr());

        // Load Mask
        if (m_mask) {
            loadMaskTexture(m_maskTextureId, m_mask->getWidth(), m_mask->getHeight(), (void *) m_mask->getDataPtr());
        }

        m_needLoading.store(0);
    }

    // Enabe FG
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_fgTextureId);

    // Enable Mask
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_maskTextureId);

    // Enable bgTexture (for read)
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, m_bgTextureId);

    // Draw
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    // Copy rendered scene to bgTexture for read in next iteration
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, 640, 480, 0);

    // Unbind bgTexture
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind Mask
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind Foreground
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_vao.release();
}

void ColorFramePainter::renderFilter()
{
    m_vao.bind();

    // Enabe FG
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_fgTextureId);

    // Enable Mask
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_maskTextureId);

    // Enable bgTexture (for read)
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, m_bgTextureId);

    // Draw
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    // Unbind bgTexture
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind Mask
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind Foreground
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_vao.release();
}

void ColorFramePainter::createFrameBuffer()
{
    m_fbo.reset(new QOpenGLFramebufferObject(640, 480));

    if (!m_fbo->isValid()) {
        qDebug() << "FBO Error";
    }

    m_fboFilter.reset(new QOpenGLFramebufferObject(640, 480));

    if (!m_fboFilter->isValid()) {
        qDebug() << "FBO Error";
    }

    // Background Texture
    glGenTextures(1, &m_bgTextureId);
    glBindTexture(GL_TEXTURE_2D, m_bgTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ColorFramePainter::enableBGRendering()
{
    m_fbo->bind();

    glViewport(0, 0, 640, 480);
    /*glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/

    m_shaderProgram->setUniformValue(m_stageUniform, 1);
}

void ColorFramePainter::enableFilterRendering()
{
    m_fboFilter->bind();

    glViewport(0, 0, 640, 480);
    /*glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/

    m_shaderProgram->setUniformValue(m_stageUniform, 2);
}

void ColorFramePainter::displayRenderedTexture()
{
    m_fboFilter->release();

    // Configure Viewport
    glViewport(0, 0, parent()->width(), parent()->height());

    m_vao.bind();

    // Enabe FG
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_fboFilter->texture());

    // Enable Mask
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_maskTextureId);

    // Enable FBO and generate Mipmap
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, m_fbo->texture());
    glGenerateMipmap(GL_TEXTURE_2D);

    m_shaderProgram->setUniformValue(m_stageUniform, 3);
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    // Unbind Background
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind Mask
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind FG
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_vao.release();
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
    m_textCoordAttr = m_shaderProgram->attributeLocation("texCoord");
    m_currentFilterUniform = m_shaderProgram->uniformLocation("currentFilter");
    m_stageUniform = m_shaderProgram->uniformLocation("stage");
    m_perspectiveMatrixUniform = m_shaderProgram->uniformLocation("perspectiveMatrix");
    m_texColorSampler = m_shaderProgram->uniformLocation("texForeground");
    m_texMaskSampler = m_shaderProgram->uniformLocation("texMask");
    m_texBackgroundSampler = m_shaderProgram->uniformLocation("texBackground");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_currentFilterUniform, m_currentFilter); // No Filter
    m_shaderProgram->setUniformValue(m_stageUniform, 1);
    m_shaderProgram->setUniformValue(m_texColorSampler, 0);
    m_shaderProgram->setUniformValue(m_texMaskSampler, 1);
    m_shaderProgram->setUniformValue(m_texBackgroundSampler, 2);
    m_shaderProgram->setUniformValue(m_perspectiveMatrixUniform, m_matrix);
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
    m_shaderProgram->enableAttributeArray(m_textCoordAttr);
    m_shaderProgram->setAttributeBuffer(m_textCoordAttr, GL_FLOAT, 0, 2 );
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

void ColorFramePainter::prepareData(shared_ptr<DataFrame> frame)
{
    m_frame = static_pointer_cast<ColorFrame>(frame);
    m_needLoading.store(1);
}

void ColorFramePainter::enableFilter(QMLEnumsWrapper::ColorFilter type)
{
    m_currentFilter = type;
}

} // End Namespace
