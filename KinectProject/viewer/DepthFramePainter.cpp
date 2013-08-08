#include "DepthFramePainter.h"
#include "types/DataInstance.h"
#include <QColor>
#include <NiTE.h>
#include "openni/OpenNIRuntime.h"
#include <QMutexLocker>
#include <cmath>

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

    // Init Vertex Buffer
    prepareVertexBuffer();
}

void DepthFramePainter::prepareData(shared_ptr<DataFrame> frame)
{
    QMutexLocker locker(&m_lockFrame);
    m_frame = static_pointer_cast<DepthFrame>(frame);
}

void DepthFramePainter::render()
{
    if (m_frame == nullptr)
        return;

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
    m_shaderProgram->setUniformValue(m_widthUniform, (float) m_frame->getWidth());
    m_shaderProgram->setUniformValue(m_heightUniform, (float) m_frame->getHeight());

    m_vao.bind();

    int count = m_frame->getWidth() * m_frame->getHeight();
    m_distancesBuffer.bind();
    m_distancesBuffer.write(0, m_frame->getDataPtr(), count * sizeof(float));
    m_distancesBuffer.release();

    glDrawArrays(GL_POINTS, m_indexAttr, count);

    // Release
    m_vao.release();
    m_shaderProgram->release();
}

void DepthFramePainter::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/glsl/depthVertex.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/glsl/depthFragment.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("colAttr", 1);
    m_shaderProgram->bindAttributeLocation("distanceAttr", 2);

    m_shaderProgram->link();

    m_indexAttr = m_shaderProgram->attributeLocation("indexAttr");
    m_distanceAttr = m_shaderProgram->attributeLocation("distanceAttr");
    m_widthUniform = m_shaderProgram->uniformLocation("width");
    m_heightUniform = m_shaderProgram->uniformLocation("height");
    m_perspectiveMatrix = m_shaderProgram->uniformLocation("perspectiveMatrix");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
    m_shaderProgram->setUniformValue(m_widthUniform, 640.0f);
    m_shaderProgram->setUniformValue(m_heightUniform, 480.0f);
    m_shaderProgram->release();
}

void DepthFramePainter::prepareVertexBuffer()
{
    float indexData[640 * 480];
    float* pData = indexData;

    for (int i=0; i<640*480; ++i) {
        *(pData++) = i;
    }

    float distanceData[640 * 480];
    pData = distanceData;

    for (int i=0; i<640*480; ++i) {
        *(pData++) = 0.0f;
    }

    m_vao.create();
    m_vao.bind();

    m_positionsBuffer.create(); // Create a vertex buffer
    m_positionsBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_positionsBuffer.bind();
    m_positionsBuffer.allocate(indexData, 640*480*sizeof(float));
    m_shaderProgram->enableAttributeArray(m_indexAttr);
    m_shaderProgram->setAttributeBuffer(m_indexAttr, GL_FLOAT, 0, 1);
    m_positionsBuffer.release();

    m_distancesBuffer.create(); // Create a vertex buffer
    m_distancesBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_distancesBuffer.bind();
    m_distancesBuffer.allocate(distanceData, 640*480*sizeof(float));
    m_shaderProgram->enableAttributeArray(m_distanceAttr);
    m_shaderProgram->setAttributeBuffer(m_distanceAttr, GL_FLOAT, 0, 1);
    m_distancesBuffer.release();

    m_vao.release();
}

} // End Namespace
