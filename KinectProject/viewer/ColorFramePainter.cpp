#include "ColorFramePainter.h"
#include <iostream>
#include <QOpenGLShaderProgram>

namespace dai {

ColorFramePainter::ColorFramePainter(StreamInstance* instance, InstanceViewer* parent)
    : ViewerPainter(instance, parent), textureUnit(0)
{
    if (instance->getType() != StreamInstance::Color) {
        std::cerr << "Invalid instance type" << std::endl;
        throw 1;
    }

    m_shaderProgram = NULL;
    m_isFrameAvailable = false;
}

ColorFramePainter::~ColorFramePainter()
{
    if (m_shaderProgram != NULL) {
        delete m_shaderProgram;
        m_shaderProgram = NULL;
    }

    m_isFrameAvailable = false;
}

void ColorFramePainter::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();

    // Create texture
    glGenTextures(1, &m_frameTexture);
}


bool ColorFramePainter::prepareNext()
{
    bool result = false;

    if (m_instance != NULL && m_instance->hasNext())
    {
        const ColorFrame& colorFrame = (const ColorFrame&) m_instance->nextFrame();;
        // FIX: Frame copy. I should not copy.
        m_frame = colorFrame;
        m_isFrameAvailable = true;
        result = true;
    }
    else if (m_instance != NULL)
    {
        m_instance->close();
        qDebug() << "Closed";
    }

    return result;
}

ColorFrame& ColorFramePainter::frame()
{
    return m_frame;
}


void ColorFramePainter::render()
{
    if (!m_isFrameAvailable)
        return;

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

    // Load into GPU
    loadVideoTexture((void *) m_frame.getDataPtr(), m_frame.getWidth(), m_frame.getHeight(), m_frameTexture);

    // Render
    glEnable(GL_TEXTURE_2D);

    m_shaderProgram->bind();
    m_shaderProgram->setAttributeArray(m_posAttr, vertexData, 3);
    m_shaderProgram->setAttributeArray(m_texCoord, texCoordsData, 2);
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_texCoord);
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_frameTexture);
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_shaderProgram->disableAttributeArray(m_texCoord);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    m_shaderProgram->release();

    glDisable(GL_TEXTURE_2D);
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

// Create Texture (overwrite previous)
void ColorFramePainter::loadVideoTexture(void* texture, GLsizei width, GLsizei height, GLuint glTextureId)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // End Namespace
