#include "Scene2DPainter.h"
#include "types/ColorFrame.h"
#include <QDebug>

namespace dai {

Scene2DPainter::Scene2DPainter()
{
}

Scene2DPainter::~Scene2DPainter()
{
    m_fboFirstPass->release();
    m_fboSecondPass->release();
    m_shaderProgram->removeAllShaders();
}

void Scene2DPainter::setMask(shared_ptr<UserFrame> mask)
{
    m_mask = mask;
}

void Scene2DPainter::initialise()
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

void Scene2DPainter::render()
{
    Q_ASSERT(m_bg->getType() == DataFrame::Color);

    // Init Each Frame (because QtQuick could change it)
    glDepthRange(0.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    // Configure ViewPort and Clear Screen
    glViewport(0, 0, m_width, m_height);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Draw Background
    if (m_bg == nullptr)
        return;

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_perspectiveMatrixUniform, m_matrix);

    // Stage 1
    enableBGRendering();
    extractBackground();// background is in bgTexture, foreground is in fgTexture
    renderBackground(); // it renders bg or fg into fboFirstPass

    // Stage 2
    renderItems(); // items first-pass rendering (fboFirstPass)

    // Stage 4
    displayRenderedTexture(); // here framebuffer (display) and second-pass rendering

    m_shaderProgram->release();
    glDisable(GL_TEXTURE_2D);

    // Restore
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
}

void Scene2DPainter::enableBGRendering()
{
    m_fboFirstPass->bind();

    glViewport(0, 0, 640, 480);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaderProgram->setUniformValue(m_stageUniform, 1);
}

void Scene2DPainter::extractBackground()
{
    m_vao.bind();

    if (m_needLoading.load())
    {
        shared_ptr<ColorFrame> frame = static_pointer_cast<ColorFrame>(m_bg);

        // Load Foreground
        ScenePainter::loadVideoTexture(m_fgTextureId, frame->getWidth(), frame->getHeight(), (void *) frame->getDataPtr());

        // Load Mask
        if (m_mask) {
            ScenePainter::loadMaskTexture(m_maskTextureId, m_mask->getWidth(), m_mask->getHeight(), (void *) m_mask->getDataPtr());
        }

        m_needLoading.store(0);
    }

    // Enabe FG
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_fgTextureId);

    // Enable Mask
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_maskTextureId);

    // Enable BG (for read)
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, m_bgTextureId);

    // Draw
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    // Copy rendered scene to bound texture (bgTexture) for read in next iteration
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

void Scene2DPainter::renderBackground()
{
    m_shaderProgram->setUniformValue(m_stageUniform, 2);
    m_vao.bind();

    glActiveTexture(GL_TEXTURE0 + 0);

    // Enable BG
    if (m_currentFilter == QMLEnumsWrapper::FILTER_INVISIBILITY) {
        glBindTexture(GL_TEXTURE_2D, m_bgTextureId);
    }
    // Enable FG
    else {
        glBindTexture(GL_TEXTURE_2D, m_fgTextureId);
    }

    // Draw
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    // Unbind FG/BG texture
    glBindTexture(GL_TEXTURE_2D, 0);

    m_vao.release();
}

void Scene2DPainter::renderItems()
{
    GLuint bg;

    if (m_currentFilter == QMLEnumsWrapper::FILTER_INVISIBILITY) {
        bg = m_bgTextureId;
    } else {
        bg = m_fgTextureId;
    }

    m_shaderProgram->release();

    // First-pass (on fboFirstPass)
    foreach (shared_ptr<SceneItem> item, m_items)
    {
        item->setBackgroundTex(bg);
        item->renderItem(1);
    }

    m_fboFirstPass->release();

    // Second-pass (write to fboSecondPass, read from fboFirstPass)
    m_fboSecondPass->bind();

    glViewport(0, 0, 640, 480);

    m_shaderProgram->bind();
    m_vao.bind();

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_fboFirstPass->texture());

    // Draw
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    m_vao.release();
    m_shaderProgram->release();

    foreach (shared_ptr<SceneItem> item, m_items)
    {
        if (item->neededPasses() == 2) {
            item->setBackgroundTex(m_fboFirstPass->texture());
            item->renderItem(2);
        }
    }

    m_fboSecondPass->release();
}

void Scene2DPainter::displayRenderedTexture()
{
    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_stageUniform, 3);

    // Configure Viewport
    glViewport(0, 0, m_width, m_height);
    m_vao.bind();

    // Enabe FG
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_fboSecondPass->texture());

    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    // Unbind FG
    glBindTexture(GL_TEXTURE_2D, 0);

    m_shaderProgram->release();
    m_vao.release();
}

void Scene2DPainter::createFrameBuffer()
{
    m_fboFirstPass.reset(new QOpenGLFramebufferObject(640, 480));

    if (!m_fboFirstPass->isValid()) {
        qDebug() << "FBO Error";
    }

    m_fboSecondPass.reset(new QOpenGLFramebufferObject(640, 480));

    if (!m_fboSecondPass->isValid()) {
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

void Scene2DPainter::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/glsl/textureVertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/glsl/textureFragment.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("texCoord", 1);

    m_shaderProgram->link();

    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_textCoordAttr = m_shaderProgram->attributeLocation("texCoord");
    m_stageUniform = m_shaderProgram->uniformLocation("stage");
    m_perspectiveMatrixUniform = m_shaderProgram->uniformLocation("perspectiveMatrix");
    m_texColorSampler = m_shaderProgram->uniformLocation("texForeground");
    m_texMaskSampler = m_shaderProgram->uniformLocation("texMask");
    m_texBackgroundSampler = m_shaderProgram->uniformLocation("texBackground");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_stageUniform, 1);
    m_shaderProgram->setUniformValue(m_texColorSampler, 0);
    m_shaderProgram->setUniformValue(m_texMaskSampler, 1);
    m_shaderProgram->setUniformValue(m_texBackgroundSampler, 2);
    m_shaderProgram->setUniformValue(m_perspectiveMatrixUniform, m_matrix);
    m_shaderProgram->release();
}

void Scene2DPainter::prepareVertexBuffer()
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

void Scene2DPainter::enableFilter(QMLEnumsWrapper::ColorFilter type)
{
    m_currentFilter = type;
}

QMLEnumsWrapper::ColorFilter Scene2DPainter::currentFilter() const
{
    return m_currentFilter;
}

} // End Namespace
