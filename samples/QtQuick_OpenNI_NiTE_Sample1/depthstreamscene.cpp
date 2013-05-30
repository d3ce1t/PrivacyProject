#include "depthstreamscene.h"
#include <cstdio>
#include <QOpenGLShaderProgram>

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
    m_nTexMapX = 0;
    m_nTexMapY = 0;
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
        m_nTexMapX = m_nativeWidth;
        m_nTexMapY = m_nativeHeight;
        m_pTexMap = new openni::RGB888Pixel[m_nTexMapX * m_nTexMapY];
    }

    // Load, compile and link the shader program
    prepareShaderProgram();

    // Prepare our geometry and associate it with shader program
    //prepareVertexBuffers();

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

/*void DepthStreamScene::prepareVertexBuffers()
{
    float vertexData[] = {
        0, 0,
        m_width, 0,
        m_width, m_height,
        0, m_height
    };

    float texCoordsData[] = {
        0, 0,
        (float)m_nativeWidth/(float)m_nTexMapX, 0,
        (float)m_nativeWidth/(float)m_nTexMapX, (float)m_nativeHeight/(float)m_nTexMapY,
        0, (float)m_nativeHeight/(float)m_nTexMapY
    };

    m_shaderProgram->bind();

    m_vertexPositionBuffer.create();
    m_vertexPositionBuffer.setUsagePattern( QOpenGLBuffer::StreamDraw );
    m_vertexPositionBuffer.bind();
    m_vertexPositionBuffer.allocate( vertexData, 4 * 2 * sizeof(float) );
    m_shaderProgram->setAttributeBuffer(m_posAttr, GL_FLOAT, 0, 2, 0);
    m_vertexPositionBuffer.release();

    m_vertexTexCoordsBuffer.create();
    m_vertexTexCoordsBuffer.setUsagePattern( QOpenGLBuffer::StreamDraw );
    m_vertexTexCoordsBuffer.bind();
    m_vertexTexCoordsBuffer.allocate( texCoordsData, 4 * 2 * sizeof(float) );
    m_shaderProgram->setAttributeBuffer(m_texCoord, GL_FLOAT, 0, 2, 0);
    m_vertexTexCoordsBuffer.release();

    m_shaderProgram->release();
}*/

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

float* DepthStreamScene::getDepthHistogram()
{
    return m_pDepthHistAux;
}

void DepthStreamScene::computeVideoTexture(nite::UserTrackerFrameRef& userTrackerFrame, openni::VideoFrameRef& colorFrame)
{
    openni::VideoFrameRef depthFrame = userTrackerFrame.getDepthFrame();

    if (depthFrame.isValid() && colorFrame.isValid()) {
        calculateHistogram(m_pDepthHist, MAX_DEPTH, depthFrame);
        openni::RGB888Pixel* texture = prepareFrameTexture(m_pTexMap, m_nTexMapX, m_nTexMapY, userTrackerFrame, colorFrame);
        loadVideoTexture(texture, m_nTexMapX, m_nTexMapY, m_frameTexture);
    }
}

void DepthStreamScene::calculateHistogram(float* pHistogram, int histogramSize, const openni::VideoFrameRef& frame)
{
    const openni::DepthPixel* pDepth = (const openni::DepthPixel*)frame.getData();

    // Calculate the accumulative histogram (the yellow display...)
    memset(pHistogram, 0, histogramSize*sizeof(float));
    memset(m_pDepthHistAux, 0, histogramSize*sizeof(float));

    int restOfRow = frame.getStrideInBytes() / sizeof(openni::DepthPixel) - frame.getWidth();
    int height = frame.getHeight();
    int width = frame.getWidth();

    unsigned int nNumberOfPoints = 0;

    // Count how may points there are in a given depth
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x, ++pDepth)
        {
            if (*pDepth != 0)
            {
                pHistogram[*pDepth]++;
                nNumberOfPoints++;
            }
        }
        // Skip rest of row (in case it exists)
        pDepth += restOfRow;
    }

    // Accumulate in the given depth all the points of previous depth layers
    for (int nIndex=1; nIndex<histogramSize; nIndex++)
    {
        pHistogram[nIndex] += pHistogram[nIndex-1];
    }

    // Normalize (0% -> 256 color value, whereas 100% -> 0 color value)
    // In other words, near objects are brighter than far objects
    if (nNumberOfPoints)
    {
        for (int nIndex=1; nIndex<histogramSize; nIndex++)
        {
            pHistogram[nIndex] = (255 * (1.0f - (pHistogram[nIndex] / nNumberOfPoints)));
        }

        memcpy(m_pDepthHistAux, pHistogram, histogramSize*sizeof(float));
    }
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

openni::RGB888Pixel* DepthStreamScene::prepareFrameTexture(openni::RGB888Pixel* texture, unsigned int width, unsigned int height,
                                                           nite::UserTrackerFrameRef& userTrackerFrame, openni::VideoFrameRef& colorFrame)
{
    openni::VideoFrameRef depthFrame = userTrackerFrame.getDepthFrame();

    memset(texture, 0, width*height*sizeof(openni::RGB888Pixel));

    // RGB Frame
    if ( colorFrame.isValid() && m_overlayMode >= 1 )
    {
        const openni::RGB888Pixel* pImageRow = (const openni::RGB888Pixel*) colorFrame.getData();
        openni::RGB888Pixel* pTexRow = m_pTexMap + colorFrame.getCropOriginY() * m_nTexMapX;
        int rowSize = colorFrame.getStrideInBytes() / sizeof(openni::RGB888Pixel);

        for (int y = 0; y < colorFrame.getHeight(); ++y)
        {
            const openni::RGB888Pixel* pImage = pImageRow;
            openni::RGB888Pixel* pTex = pTexRow + colorFrame.getCropOriginX();

            for (int x = 0; x < colorFrame.getWidth(); ++x, ++pImage, ++pTex)
            {
                *pTex = *pImage;
            }

            pImageRow += rowSize;
            pTexRow += m_nTexMapX;
        }
    }

    // Depth Frame
    if (depthFrame.isValid() && m_overlayMode <= 1)
    {
        const nite::UserMap& userLabels = userTrackerFrame.getUserMap();
        float factor[3] = {1, 1, 1};
        const nite::UserId* pLabels = userLabels.getPixels(); // Each Pixel has a label
        const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*) depthFrame.getData();
        openni::RGB888Pixel* pTexRow = texture + depthFrame.getCropOriginY() * width;
        int rowSize = depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel); // why not depthFrame.getWidth()?

        for (int y = 0; y < depthFrame.getHeight(); ++y)
        {
            const openni::DepthPixel* pDepth = pDepthRow;
            openni::RGB888Pixel* pTex = pTexRow + depthFrame.getCropOriginX();

            for (int x = 0; x < depthFrame.getWidth(); ++x, ++pDepth, ++pTex, ++pLabels)
            {
                if (*pDepth != 0)
                {
                    if (*pLabels == 0) {
                        if (!g_drawBackground) {
                            factor[0] = factor[1] = factor[2] = 0;
                        } else {
                            factor[0] = colors[colorCount].x(); // 100%
                            factor[1] = colors[colorCount].y(); // 100%
                            factor[2] = colors[colorCount].z(); // 100%
                        }
                    } else {
                        factor[0] = colors[*pLabels % colorCount].x();
                        factor[1] = colors[*pLabels % colorCount].y();
                        factor[2] = colors[*pLabels % colorCount].z();
                    }

                    int nHistValue = m_pDepthHist[*pDepth]; // Get a Color
                    pTex->r = nHistValue*factor[0];
                    pTex->g = nHistValue*factor[1];
                    pTex->b = nHistValue*factor[2];

                    factor[0] = factor[1] = factor[2] = 1;
                }
            }

            pDepthRow += rowSize;
            pTexRow += width;
        }
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
