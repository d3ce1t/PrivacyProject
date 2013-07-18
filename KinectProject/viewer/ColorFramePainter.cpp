#include "ColorFramePainter.h"
#include "types/UserFrame.h"

namespace dai {

ColorFramePainter::ColorFramePainter(QOpenGLContext *context)
    : Painter(context), textureUnit(0)
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

    // Load into GPU
    if (m_mask)
        loadMaskTexture((void *) m_mask->getDataPtr(), m_mask->getWidth(), m_mask->getHeight(), m_maskTexture);

    loadVideoTexture((void *) m_frame->getDataPtr(), m_frame->getWidth(), m_frame->getHeight(), m_foregroundTexture);

    // Render
    m_shaderProgram->bind();
    m_vao.bind();

    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_foregroundTexture);
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    m_vao.release();
    m_shaderProgram->release();
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
    m_perspectiveMatrix = m_shaderProgram->uniformLocation("perspectiveMatrix");
    m_texSampler = m_shaderProgram->uniformLocation("texSampler");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_texSampler, 0);
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
    m_shaderProgram->release();
}

void ColorFramePainter::prepareVertexBuffer()
{
    float vertexData[] = {
        -1.0, 1.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, -1.0, 0.0,
        -1.0, -1.0, 0.0
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
void ColorFramePainter::loadVideoTexture(void* texture, GLsizei width, GLsizei height, GLuint glTextureId)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Create Texture (overwrite previous)
void ColorFramePainter::loadMaskTexture(void* texture, GLsizei width, GLsizei height, GLuint glTextureId)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // End Namespace
