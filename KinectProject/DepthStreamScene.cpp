#include "DepthStreamScene.h"
#include <cstdio>
#include <QOpenGLShaderProgram>
#include "functions.h"

DepthStreamScene::DepthStreamScene()
    : textureUnit(0), colorCount(3)
{
    colors[0] = QVector3D(1, 0, 0); // Red
    colors[1] = QVector3D(0, 1, 0); // Green
    colors[2] = QVector3D(0, 0, 1); // Blue
    colors[3] = QVector3D(1, 1, 1); // White

    m_shaderProgram = NULL;
    m_pTexMap = NULL;
    m_nativeWidth = 0;
    m_nativeHeight = 0;
    g_drawBackground = true;
    m_overlayMode = 0;
}

DepthStreamScene::~DepthStreamScene()
{
    if (m_pTexMap != NULL) {
        delete[] m_pTexMap;
        m_pTexMap = NULL;
    }
}

void DepthStreamScene::setNativeResolution(int width, int height)
{
    m_nativeWidth = width;
    m_nativeHeight = height;
}

void DepthStreamScene::initialise()
{
    if (m_nativeWidth == 0 || m_nativeHeight == 0)
        throw 0;

    // Texture map init
    if (m_pTexMap == NULL) {
        m_pTexMap = new openni::RGB888Pixel[m_nativeWidth * m_nativeHeight];
    }

    // Load, compile and link the shader program
    prepareShaderProgram();

    // Create texture
    glGenTextures(1, &m_frameTexture);
}

void DepthStreamScene::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/textureVertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/textureFragment.fsh");
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

void DepthStreamScene::update(float t)
{
    Q_UNUSED(t);
}

void DepthStreamScene::render()
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

    /*float texCoordsData[] = {
        0, 0,
        (float)m_nativeWidth/(float)m_nTexMapX, 0,
        (float)m_nativeWidth/(float)m_nTexMapX, (float)m_nativeHeight/(float)m_nTexMapY,
        0, (float)m_nativeHeight/(float)m_nTexMapY
    };*/

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

void DepthStreamScene::resize( float w, float h )
{
    m_width = w;
    m_height = h;
}


void DepthStreamScene::computeVideoTexture(const dai::DepthFrame& frame)
{
    dai::calculateHistogram(m_pDepthHist, MAX_DEPTH, frame);
    openni::RGB888Pixel* texture = prepareFrameTexture(m_pTexMap, frame);
    loadVideoTexture(texture, frame.getWidth(), frame.getHeight(), m_frameTexture);
}


// Create Texture (overwrite previous)
void DepthStreamScene::loadVideoTexture(openni::RGB888Pixel* texture, GLsizei width, GLsizei height, GLuint glTextureId)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

openni::RGB888Pixel* DepthStreamScene::prepareFrameTexture(openni::RGB888Pixel* texture, const dai::DepthFrame& frame)
{
    memset(texture, 0, frame.getWidth()*frame.getHeight()*sizeof(openni::RGB888Pixel));
    float factor[3] = {1, 1, 1};
    openni::RGB888Pixel* pTexRow = texture;

    for (int y = 0; y < frame.getHeight(); ++y)
    {
        openni::RGB888Pixel* pTex = pTexRow;

        for (int x = 0; x < frame.getWidth(); ++x, ++pTex)
        {
            int distance = frame.getItem(y, x);

            if (distance != 0)
            {
                factor[0] = colors[colorCount].x(); // 100%
                factor[1] = colors[colorCount].y(); // 100%
                factor[2] = colors[colorCount].z(); // 100%


                int nHistValue = m_pDepthHist[distance]; // Get a Color
                pTex->r = nHistValue*factor[0];
                pTex->g = nHistValue*factor[1];
                pTex->b = nHistValue*factor[2];

                factor[0] = factor[1] = factor[2] = 1;
            }
        }

        pTexRow += frame.getWidth();
    }

    return texture;
}

void DepthStreamScene::setMatrix(QMatrix4x4 &matrix) {
    this->m_matrix = matrix;
}

void DepthStreamScene::setDrawBackgroundFlag(bool value)
{
    g_drawBackground = value;
    emit changeOfStatus();
}

void DepthStreamScene::setOverlayMode(int value)
{
    m_overlayMode = value % 3;
    emit changeOfStatus();
}
