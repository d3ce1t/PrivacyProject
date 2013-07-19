#include "ColorFramePainter.h"
#include "types/UserFrame.h"

namespace dai {

ColorFramePainter::ColorFramePainter(QOpenGLContext *context)
    : Painter(context)
{
    m_frame = nullptr;
    m_textureData.reset(new u_int8_t[640*480]);
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

    // Create empty texture for Background
    glGenTextures(1, &m_backgroundTexture);
    /*glBindTexture(GL_TEXTURE_2D, m_backgroundTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);*/
}

ColorFrame& ColorFramePainter::frame()
{
    return *m_frame;
}

void ColorFramePainter::prepareData(shared_ptr<DataFrame> frame)
{
    m_frame = static_pointer_cast<ColorFrame>(frame);

    if (m_mask)
    {
        memset(m_textureData.get(), 0, 640*480*sizeof(u_int8_t));

        for (int i=0; i<480; ++i) {
            for (int j=0; j<640; ++j) {
                u_int8_t label = m_mask->getItem(i,j);
                if (label > 0) {
                    m_textureData.get()[i*640+j] = 255;
                }
            }
        }
    }
}

void ColorFramePainter::render()
{
    if (m_frame == nullptr)
        return;

    // Load into GPU
    if (m_mask) {
        // Get initial background at 20th frame
        if (m_frame->getIndex() == 20) {
            loadVideoTexture(m_backgroundTexture, m_frame->getWidth(), m_frame->getHeight(), (void *) m_frame->getDataPtr());
        }

        // Load Mask
        loadMaskTexture(m_maskTexture, m_mask->getWidth(), m_mask->getHeight(), (void *) m_textureData.get());
    }

    // Load Foreground
    loadVideoTexture(m_foregroundTexture, m_frame->getWidth(), m_frame->getHeight(), (void *) m_frame->getDataPtr());

    // Render
    m_shaderProgram->bind();
    m_vao.bind();

    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);

    // Bind Foreground
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_foregroundTexture);

    // Bind Mask
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_maskTexture);

    // Bind Background
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, m_backgroundTexture);

    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    // Unbind Background
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind Mask
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Unbind Foreground
    glActiveTexture(GL_TEXTURE0 + 0);
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
void ColorFramePainter::loadVideoTexture(GLuint glTextureId, GLsizei width, GLsizei height, void* texture)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Create Texture (overwrite previous)
void ColorFramePainter::loadMaskTexture(GLuint glTextureId, GLsizei width, GLsizei height, void* texture)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // End Namespace
