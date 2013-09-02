#include "SilhouetteItem.h"
#include "viewer/ScenePainter.h"

namespace dai {

RGBColor SilhouetteItem::staticUserColors[USER_COLORS] = {
    {  0,   0,   0},
    {255,   0,   0},
    {  0, 255,   0},
    {  0,   0, 255},
    {255, 255,   0},
    {  0, 255, 255}
};

SilhouetteItem::SilhouetteItem()
    : textureUnit(0)
{
    m_user = nullptr;
    m_textureMask = new RGBColor[640*480];
}

void SilhouetteItem::setUser(shared_ptr<UserFrame> user)
{
    m_user = user;

    for (int i=0; i<480; ++i) {
        for (int j=0; j<640; ++j) {
            short userColor = m_user->getItem(i,j) % USER_COLORS;
            m_textureMask[i*640+j] = staticUserColors[userColor];
        }
    }
}

void SilhouetteItem::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();

    // Init Vertex Buffer
    prepareVertexBuffer();

    // Create texture
    glGenTextures(1, &m_frameTexture);
}

void SilhouetteItem::render()
{
    if (m_user == nullptr)
        return;

    glEnable(GL_BLEND);

    // Load into GPU
    loadVideoTexture((void *) m_textureMask, m_user->getWidth(), m_user->getHeight(), m_frameTexture);

    // Render
    m_shaderProgram->bind();
    m_vao.bind();

    m_shaderProgram->setUniformValue(m_perspectiveMatrix, scene()->getMatrix());

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_frameTexture);
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    m_vao.release();
    m_shaderProgram->release();

    glDisable(GL_BLEND);
}

void SilhouetteItem::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/glsl/userVertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/glsl/userFragment.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("texCoord", 1);

    m_shaderProgram->link();

    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_texCoord = m_shaderProgram->attributeLocation("texCoord");
    m_perspectiveMatrix = m_shaderProgram->uniformLocation("perspectiveMatrix");
    m_texSampler = m_shaderProgram->uniformLocation("texSampler");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_texSampler, 0);
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, scene()->getMatrix());
    m_shaderProgram->release();
}

void SilhouetteItem::prepareVertexBuffer()
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
void SilhouetteItem::loadVideoTexture(void* texture, GLsizei width, GLsizei height, GLuint glTextureId)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // End Namespace