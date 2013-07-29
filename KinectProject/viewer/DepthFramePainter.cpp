#include "DepthFramePainter.h"
#include "../dataset/DataInstance.h"
/*#include <cstdio>
#include "KMeans.h"
#include "DepthSeg.h"
#include "Utils.h"*/
#include <QColor>
#include <NiTE.h>
#include "openni/OpenNIRuntime.h"
#include <QMutexLocker>

namespace dai {

DepthFramePainter::DepthFramePainter(InstanceViewer *parent)
    : Painter(parent)
{
    m_frame = nullptr;
}

DepthFramePainter::~DepthFramePainter()
{
    m_frame = nullptr;
}

void DepthFramePainter::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();
}

DepthFrame& DepthFramePainter::frame()
{
    QMutexLocker locker(&m_lockFrame);
    return *m_frame;
}

void DepthFramePainter::prepareData(shared_ptr<DataFrame> frame)
{
    QMutexLocker locker(&m_lockFrame);
    m_frame = static_pointer_cast<DepthFrame>(frame);
    DepthFrame::calculateHistogram(m_pDepthHist, *m_frame);
}

void DepthFramePainter::render()
{
    QMutexLocker locker(&m_lockFrame);

    if (m_frame == nullptr)
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

    float* vertex = new float[m_frame->getNumOfNonZeroPoints() * 3];
    float* color = new float[m_frame->getNumOfNonZeroPoints() * 3];
    int offset = 0;

    for (int y = 0; y < m_frame->getHeight(); ++y)
    {
        for (int x = 0; x < m_frame->getWidth(); ++x)
        {
            float distance = m_frame->getItem(y, x);

            if (distance > 0)
            {
                float normX;// = DataInstance::normalise(x, 0, m_frame->getWidth()-1, -1, 1);
                float normY;// = DataInstance::normalise(y, 0, m_frame->getHeight()-1, -1, 1);
                float norm_color = DataInstance::normalise(distance, 0, 6, 0, 0.83);
                convertDepthToRealWorld(x, y, distance, normX, normY);

                if (norm_color > 0.83)
                    norm_color = 0.83;

                vertex[offset] = normX;
                vertex[offset+1] = -normY;
                vertex[offset+2] = -distance;

                QColor depthColor = QColor::fromHsvF(norm_color, 1.0, 1.0);
                color[offset] = depthColor.redF();
                color[offset+1] = depthColor.greenF();
                color[offset+2] = depthColor.blueF();

                /*color[offset] = m_pDepthHist[distance];
                color[offset+1] = m_pDepthHist[distance];
                color[offset+2] = m_pDepthHist[distance];*/

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

    glDrawArrays(GL_POINTS, m_posAttr, m_frame->getNumOfNonZeroPoints());

    // Release
    m_shaderProgram->disableAttributeArray(m_colorAttr);
    m_shaderProgram->disableAttributeArray(m_posAttr);
    m_shaderProgram->release();

    delete[] vertex;
    delete[] color;
    //delete[] data;

    m_frame = nullptr;

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

void DepthFramePainter::convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY) {

    static const double fx_d = 1.0 / 5.9421434211923247e+02;
    static const double fy_d = 1.0 / 5.9104053696870778e+02;
    static const double cx_d = m_frame->getWidth() / 2; // 3.3930780975300314e+02;
    static const double cy_d = m_frame->getHeight() / 2; // 2.4273913761751615e+02;

    outX = float((x - cx_d) * distance * fx_d);
    outY = float((y - cy_d) * distance * fy_d);

    //qDebug() << outX << outY;
}

} // End Namespace
