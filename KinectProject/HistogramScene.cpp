#include "HistogramScene.h"
#include <QOpenGLShaderProgram>

namespace dai {

HistogramScene::HistogramScene()
{
    m_shaderProgram = NULL;
}

HistogramScene::~HistogramScene()
{
    if (m_shaderProgram != NULL)
    {
        delete m_shaderProgram;
        m_shaderProgram = NULL;
    }
}

void HistogramScene::initialise()
{
    prepareShaderProgram();

    // Create texture
    glGenTextures(1, &m_frameTexture);
}

void HistogramScene::update(float t)
{

}

void HistogramScene::render()
{
    float vertexData[] = {
        -1.0, 1.0, 0.2,
        0.0, 1.0, 0.2,
        0.0, 0.0, 0.2,
        -1.0, 0.0, 0.2
    };

    float texCoordsData[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };

    glBindTexture(GL_TEXTURE_2D, m_frameTexture);
    //glTexParameteri(GL_TEXTURE_2D, GL_GEN, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_UNSIGNED_BYTE, m_histogram);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Render
    glEnable(GL_TEXTURE_2D);

    m_shaderProgram->bind();
    m_shaderProgram->setAttributeArray(m_posAttr, vertexData, 3);
    m_shaderProgram->setAttributeArray(m_texCoord, texCoordsData, 2);
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_texCoord);
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_frameTexture);
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_shaderProgram->disableAttributeArray(m_texCoord);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    m_shaderProgram->release();

    glDisable(GL_TEXTURE_2D);
}

void HistogramScene::resize( float w, float h )
{

}

void HistogramScene::prepareShaderProgram()
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

void HistogramScene::setMatrix(QMatrix4x4& matrix) {
    this->m_matrix = matrix;
}

void HistogramScene::setHistogram(float histogram[])
{
    for (int i=0; i<100; ++i) {
        for (int j=0; j<100; ++j) {
            m_histogram[i][j] = (GLubyte) histogram[i*100 + j];
        }
    }
}

} // End Namespace
