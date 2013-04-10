#include "ColorFramePainter.h"
#include <cstdio>
#include <QOpenGLShaderProgram>

namespace dai {

ColorFramePainter::ColorFramePainter()
    : textureUnit(0)
{
    colors[0] = QVector3D(1, 1, 1); // White

    m_shaderProgram = NULL;
    m_pTexMap = NULL;
    m_pTexMapWidth = 0;
    m_pTexMapHeight = 0;
    m_isFrameAvailable = false;
}

ColorFramePainter::~ColorFramePainter()
{
    if (m_pTexMap != NULL) {
        delete[] m_pTexMap;
        m_pTexMap = NULL;
    }

    if (m_shaderProgram != NULL) {
        delete m_shaderProgram;
        m_shaderProgram = NULL;
    }

    m_pTexMapWidth = 0;
    m_pTexMapHeight = 0;
    m_isFrameAvailable = false;
}

void ColorFramePainter::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();

    // Create texture
    glGenTextures(1, &m_frameTexture);
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

    loadVideoTexture(m_pTexMap, m_pTexMapWidth, m_pTexMapHeight, m_frameTexture);


   /* for (int y = 0; y < m_frame.getHeight(); ++y)
    {
        for (int x = 0; x < m_frame.getWidth(); ++x)
        {
            int distance = frame.getItem(y, x);

            if (distance != 0)
            {
                int nHistValue = m_pDepthHist[distance]; // Get a Color

                float color[] = {
                    nHistValue, nHistValue, nHistValue
                };

                float vertice[] = {
                    x, y, distance
                };




            }
        }
    }*/

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

void ColorFramePainter::resize( float w, float h )
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

void ColorFramePainter::setFrame(const DataFrame &frame)
{
    const DepthFrame& depthFrame = static_cast<const DepthFrame&>(frame);

    m_frame = depthFrame;

    // Texture map init (preserve memory if size matches)
    if (m_pTexMap == NULL || m_pTexMapWidth != depthFrame.getWidth() || m_pTexMapHeight != depthFrame.getHeight())
    {
        if (m_pTexMap != NULL) {
            delete[] m_pTexMap;
        }

        m_pTexMapWidth = depthFrame.getWidth();
        m_pTexMapHeight = depthFrame.getHeight();
        m_pTexMap = new openni::RGB888Pixel[m_pTexMapWidth * m_pTexMapHeight];
    }

    //DepthFrame::calculateHistogram(m_pDepthHist /*, MAX_DEPTH, depthFrame);
    prepareFrameTexture(m_pTexMap, depthFrame);
    m_isFrameAvailable = true;
}

void ColorFramePainter::prepareShaderProgram()
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

void ColorFramePainter::prepareFrameTexture(openni::RGB888Pixel* texture, const dai::DepthFrame& frame)
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
                factor[0] = colors[0].x(); // 100%
                factor[1] = colors[0].y(); // 100%
                factor[2] = colors[0].z(); // 100%


                int nHistValue = m_pDepthHist[distance]; // Get a Color
                pTex->r = nHistValue*factor[0];
                pTex->g = nHistValue*factor[1];
                pTex->b = nHistValue*factor[2];

                factor[0] = factor[1] = factor[2] = 1;
            }
        }

        pTexRow += frame.getWidth();
    }
}

// Create Texture (overwrite previous)
void ColorFramePainter::loadVideoTexture(openni::RGB888Pixel* texture, GLsizei width, GLsizei height, GLuint glTextureId)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // End Namespace
