#include "DepthFramePainter.h"
#include <cstdio>
#include <QOpenGLShaderProgram>

namespace dai {

DepthFramePainter::DepthFramePainter()
{
    colors[0] = QVector3D(1, 1, 1); // White

    m_shaderProgram = NULL;
    m_isFrameAvailable = false;
}

DepthFramePainter::~DepthFramePainter()
{
    if (m_shaderProgram != NULL) {
        delete m_shaderProgram;
        m_shaderProgram = NULL;
    }

    m_isFrameAvailable = false;
}

void DepthFramePainter::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();
}

float DepthFramePainter::normalise(float value, float minValue, float maxValue, float newMax, float newMin)
{
    return ( (value - minValue) * (newMax - newMin) ) / (maxValue - minValue) + newMin;
}

void DepthFramePainter::render()
{
    if (!m_isFrameAvailable)
        return;

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    //glDisable(GL_DEPTH_TEST);

    // Bind Shader
    m_shaderProgram->bind();

    int minValue = DepthFrame::minValue(m_frame);
    int maxValue = DepthFrame::maxValue(m_frame);

    qDebug() << "Depth Min value: " << minValue;
    qDebug() << "Depth Max value: " << maxValue;

    for (int y = 0; y < m_frame.getHeight(); ++y)
    {
        for (int x = 0; x < m_frame.getWidth(); ++x)
        {
            int distance = m_frame.getItem(y, x);

            if (distance > 0)
            {
                float normDistance = normalise(distance, minValue, maxValue, 1, -1);
                float normX = normalise(x, 0, m_frame.getWidth()-1, 1, -1);
                float normY = normalise(y, 0, m_frame.getHeight()-1, 1, -1);
                float nHistValue = m_pDepthHist[distance]; // Get a Color

                float color[] = {
                    nHistValue, nHistValue, nHistValue
                };

                float vertex[] = {
                    normX, -normY, -normDistance
                };

                m_shaderProgram->setAttributeArray(m_posAttr, vertex, 3);
                m_shaderProgram->setAttributeArray(m_colorAttr, color, 3);
                m_shaderProgram->setUniformValue(m_pointSize, 2.0f);
                m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
                m_shaderProgram->enableAttributeArray(m_posAttr);
                m_shaderProgram->enableAttributeArray(m_colorAttr);

                glDrawArrays(GL_POINTS, m_posAttr, 1);
            }
        }
    }

    // Release
    m_shaderProgram->disableAttributeArray(m_colorAttr);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    m_shaderProgram->release();

    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

void DepthFramePainter::resize( float w, float h )
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

void DepthFramePainter::setFrame(const DataFrame &frame)
{
    const DepthFrame& depthFrame = static_cast<const DepthFrame&>(frame);
    m_frame = depthFrame;
    DepthFrame::calculateHistogram(m_pDepthHist, MAX_DEPTH, depthFrame);
    m_isFrameAvailable = true;
}

void DepthFramePainter::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/simpleVertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/simpleFragment.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("colAttr", 1);

    m_shaderProgram->link();

    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_colorAttr = m_shaderProgram->attributeLocation("colAttr");
    m_pointSize = m_shaderProgram->uniformLocation("sizeAttr");
    m_perspectiveMatrix = m_shaderProgram->uniformLocation("perspectiveMatrix");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
    m_shaderProgram->setUniformValue(m_pointSize, 2.0f);
    m_shaderProgram->release();
}

} // End Namespace
