#include "SilhouetteItem.h"
#include "viewer/ScenePainter.h"

namespace dai {

SilhouetteItem::SilhouetteItem()
    : SceneItem(ITEM_SILHOUETTE)
{
    m_user = nullptr;
    m_neededPasses = 2;
}

void SilhouetteItem::setUser(shared_ptr<UserFrame> user)
{
    m_user = user;
}

void SilhouetteItem::setDrawingEffect(SilhouetteEffect effect)
{
    m_drawingEffect = effect;
}

void SilhouetteItem::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();

    // Init Vertex Buffer
    prepareVertexBuffer();

    // Create texture
    glGenTextures(1, &m_maskTextureId);
}

void SilhouetteItem::render(int pass)
{
    if (m_user == nullptr)
        return;

    if (pass == 1) {
        renderFirstPass();
    }
    else if (pass == 2) {
        renderSecondPass();
    }
}

void SilhouetteItem::renderFirstPass()
{
    // Load into GPU
    m_scene->loadMaskTexture(m_maskTextureId, m_user->getWidth(), m_user->getHeight(), (void *) m_user->getDataPtr());

    // Render
    m_shaderProgram->bind();
    m_vao.bind();

    m_shaderProgram->setUniformValue(m_stageUniform, 1);
    m_shaderProgram->setUniformValue(m_silhouetteEffectUniform, m_drawingEffect);

    // Enable FG
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_fgTextureId);

    // Enable Mask (unit 0 is reserved for FG)
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_maskTextureId);

    // Draw
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    // Unbind Mask
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind Foreground
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_vao.release();
    m_shaderProgram->release();
}

void SilhouetteItem::renderSecondPass()
{
    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_stageUniform, 2);
    m_vao.bind();

    // Enable FG
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_fgTextureId);

    // Enable Mask (unit 0 is reserved for FG)
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_maskTextureId);

    // Draw
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    // Unbind Mask
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind Foreground
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_vao.release();
    m_shaderProgram->release();
}

void SilhouetteItem::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/glsl/silhouette.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/glsl/silhouette.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("texCoord", 1);

    m_shaderProgram->link();

    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_texCoord = m_shaderProgram->attributeLocation("texCoord");
    m_silhouetteEffectUniform = m_shaderProgram->uniformLocation("silhouetteEffect");
    m_stageUniform = m_shaderProgram->uniformLocation("stage");
    m_texFGSampler = m_shaderProgram->uniformLocation("texForeground");
    m_texMaskSampler = m_shaderProgram->uniformLocation("texMask");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_silhouetteEffectUniform, EFFECT_NORMAL); // No Filter
    m_shaderProgram->setUniformValue(m_stageUniform, 1);
    m_shaderProgram->setUniformValue(m_texFGSampler, 0);
    m_shaderProgram->setUniformValue(m_texMaskSampler, 1);
    m_shaderProgram->release();
}

void SilhouetteItem::prepareVertexBuffer()
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

} // End Namespace
