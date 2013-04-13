#include "DepthFramePainter.h"
#include <cstdio>
#include <QOpenGLShaderProgram>
#include "dataset/DataInstance.h"

namespace dai {

DepthFramePainter::DepthFramePainter(DataInstance* instance)
    : ViewerPainter(instance)
{
    if (instance->getMetadata().getType() != InstanceInfo::Depth)
        throw 1;

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

    qDebug() << "DepthFramePaiter destroyed";
}

void DepthFramePainter::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();
}

bool DepthFramePainter::prepareNext()
{
    bool result = false;

    if (m_instance != NULL && m_instance->hasNext())
    {
        const dai::DataFrame& frame = m_instance->nextFrame();
        const DepthFrame& depthFrame = static_cast<const DepthFrame&>(frame);
        // FIX: Frame copy. I should not copy.
        m_frame = depthFrame;
        DepthFrame::calculateHistogram(m_pDepthHist, depthFrame);
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

void DepthFramePainter::render()
{
    if (!m_isFrameAvailable)
        return;

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    //glDisable(GL_DEPTH_TEST);

    static float min = DepthFrame::minValue(m_frame);
    static float max = DepthFrame::maxValue(m_frame);

    /*qDebug() << "Min: " << min;
    qDebug() << "Max: " << max;*/

    // Bind Shader
    m_shaderProgram->bind();

    float* vertex = new float[m_frame.getNumberOfNonZeroPoints() * 3 * sizeof(float)];
    float* color = new float[m_frame.getNumberOfNonZeroPoints() * 3 * sizeof(float)];
    int index = 0;

    for (int y = 0; y < m_frame.getHeight(); ++y)
    {
        for (int x = 0; x < m_frame.getWidth(); ++x)
        {
            float distance = m_frame.getItem(y, x);

            if (distance > 0)
            {
                float normX = DataInstance::normalise(x, 0, m_frame.getWidth()-1, -1, 1);
                float normY = DataInstance::normalise(y, 0, m_frame.getHeight()-1, -1, 1);

                vertex[index] = normX;
                vertex[index+1] = -normY;
                vertex[index+2] = -distance;

                color[index] = m_pDepthHist[distance];
                color[index+1] = m_pDepthHist[distance];
                color[index+2] = m_pDepthHist[distance];

                index+=3;
            }
        }
    }

    m_shaderProgram->setAttributeArray(m_posAttr, vertex, 3);
    m_shaderProgram->setAttributeArray(m_colorAttr, color, 3);
    m_shaderProgram->setUniformValue(m_pointSize, 2.0f);
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_colorAttr);

    glDrawArrays(GL_POINTS, m_posAttr, m_frame.getNumberOfNonZeroPoints());

    // Release
    m_shaderProgram->disableAttributeArray(m_colorAttr);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    m_shaderProgram->release();

    delete[] vertex;
    delete[] color;

    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

void DepthFramePainter::resize( float w, float h )
{
    Q_UNUSED(w);
    Q_UNUSED(h);
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
