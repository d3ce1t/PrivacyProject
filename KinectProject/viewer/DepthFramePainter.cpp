#include "DepthFramePainter.h"
#include <cstdio>
#include <iostream>
#include <QOpenGLShaderProgram>
//#include <qopenglext.h>
#include "../dataset/DataInstance.h"
#include "KMeans.h"
#include "DepthSeg.h"
#include "Utils.h"
#include <QColor>

namespace dai {

const QVector3D DepthFramePainter::m_colors[5] = {
    QVector3D(1.0, 0.0, 0.0),
    QVector3D(0.0, 1.0, 0.0),
    QVector3D(0.0, 0.0, 1.0),
    QVector3D(1.0, 1.0, 0.0),
    QVector3D(0.0, 1.0, 1.0)
};

DepthFramePainter::DepthFramePainter(StreamInstance *instance, InstanceViewer *parent)
    : ViewerPainter(instance, parent)
{
    if (instance->getType() != StreamInstance::Depth) {
        std::cerr << "Invalid instance type" << std::endl;
        throw 1;
    }

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

bool DepthFramePainter::prepareNext()
{
    bool result = false;

    if (m_instance != NULL && m_instance->hasNext())
    {
        const DepthFrame& depthFrame = (const DepthFrame&) m_instance->nextFrame();
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

DepthFrame& DepthFramePainter::frame()
{
    return m_frame;
}

void DepthFramePainter::render()
{
    if (!m_isFrameAvailable)
        return;

    //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    //glDisable(GL_DEPTH_TEST);

    // Filter data
    /*float* data = new float[m_frame.getNumOfNonZeroPoints()];
    int index = 0;

    for (int i=0; i<m_frame.getHeight(); ++i) {
        for (int j=0; j<m_frame.getWidth(); ++j) {
            float value = m_frame.getItem(i, j);
            if (value != 0) {
                data[index] = value;
                index++;
            }
        }
    }

    float min_distance = dai::min_element(data, m_frame.getNumOfNonZeroPoints());
    float max_distance = dai::max_element(data, m_frame.getNumOfNonZeroPoints());*/


    //float max_cluster = 3;
    //const KMeans* kmeans = KMeans::execute(data, m_frame.getNumOfNonZeroPoints(), max_cluster);
    //DepthSeg* dseg = new DepthSeg(m_frame);
    //dseg->execute();
    //float max_cluster = dai::max_element(dseg->getClusterMask(), m_frame.getWidth() * m_frame.getHeight());

    // Bind Shader
    m_shaderProgram->bind();

    float* vertex = new float[m_frame.getNumOfNonZeroPoints() * 3];
    float* color = new float[m_frame.getNumOfNonZeroPoints() * 3];

    int offset = 0;

    for (int y = 0; y < m_frame.getHeight(); ++y)
    {
        for (int x = 0; x < m_frame.getWidth(); ++x)
        {
            float distance = m_frame.getItem(y, x);

            if (distance > 0)
            {                
                float normX = DataInstance::normalise(x, 0, m_frame.getWidth()-1, -1, 1);
                float normY = DataInstance::normalise(y, 0, m_frame.getHeight()-1, -1, 1);
                //float norm_color = DataInstance::normalise(distance, min_distance, max_distance, 0, 0.83);

                vertex[offset] = normX;
                vertex[offset+1] = -normY;
                vertex[offset+2] = -distance;

                //float cluster = dseg->getCluster(y, x);

                /*if (cluster != -1) {
                    float norm_color = DataInstance::normalise(cluster / max_cluster, 0, 1, 0, 0.83);
                    QColor cluster_color = QColor::fromHsvF(norm_color, 1.0, 1.0);
                    color[offset] = cluster_color.redF();
                    color[offset+1] = cluster_color.greenF();
                    color[offset+2] = cluster_color.blueF();
                }*/
                /*else {*/

                short int label = m_frame.getLabel(y, x);

                if (label == 0) {
                    color[offset] = m_pDepthHist[distance];
                    color[offset+1] = m_pDepthHist[distance];
                    color[offset+2] = m_pDepthHist[distance];
                } else {
                    color[offset] = m_colors[label-1 % 5].x();
                    color[offset+1] = m_colors[label-1 % 5].y();
                    color[offset+2] = m_colors[label-1 % 5].z();
                }

                offset+=3;
            }
        }
    }

    m_shaderProgram->setAttributeArray(m_posAttr, vertex, 3);
    m_shaderProgram->setAttributeArray(m_colorAttr, color, 3);
    m_shaderProgram->setUniformValue(m_pointSize, 2.0f);
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->enableAttributeArray(m_colorAttr);

    glDrawArrays(GL_POINTS, m_posAttr, m_frame.getNumOfNonZeroPoints());

    // Release
    m_shaderProgram->disableAttributeArray(m_colorAttr);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    m_shaderProgram->release();

    delete[] vertex;
    delete[] color;
    //delete[] data;

    //glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

void DepthFramePainter::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/glsl/simpleVertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/glsl/simpleFragment.fsh");
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
