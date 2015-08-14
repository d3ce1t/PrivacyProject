#include "Scene3DPainter.h"
#include "types/DepthFrame.h"
#include <QDebug>

namespace dai {

Scene3DPainter::Scene3DPainter()
{
}

void Scene3DPainter::initialise()
{
    // Load, compile and link the shader program
    prepareShaderProgram();

    // Init Vertex Buffer
    prepareVertexBuffer();
}

void Scene3DPainter::render(QOpenGLFramebufferObject *target)
{
    Q_ASSERT(m_bg == nullptr || m_bg->getType() == DataFrame::Depth);

    if (target)
        target->bind();

    // Init Each Frame (because QtQuick could change it)
    glDepthRange(0.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    // Configure ViewPort and Clear Screen
    glViewport(0, 0, m_scene_width, m_scene_height);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Draw Background
    if (m_bg)
    {
        shared_ptr<DepthFrame> bgFrame = static_pointer_cast<DepthFrame>(m_bg);

        m_shaderProgram->bind();
        m_shaderProgram->setUniformValue(m_perspectiveMatrix, m_matrix);
        m_shaderProgram->setUniformValue(m_widthUniform, (float) bgFrame->width());
        m_shaderProgram->setUniformValue(m_heightUniform, (float) bgFrame->height());

        m_vao.bind();
        int count = bgFrame->width() * bgFrame->height();

        m_distancesBuffer.bind();
        m_distancesBuffer.write(0, bgFrame->getDataPtr(), count * sizeof(float));
        m_distancesBuffer.release();

        glDrawArrays(GL_POINTS, m_indexAttr, count);

        // Release
        m_vao.release();
        m_shaderProgram->release();
    }

    renderItems();

    // Restore
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    if (target)
        target->release();
}

void Scene3DPainter::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/glsl/scene3d.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/glsl/scene3d.fsh");
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

void Scene3DPainter::prepareVertexBuffer()
{
    // float indexData[640 * 480]; // When built with msvc 11 it triggers a stack overflow at this line
    float* indexData = new float[640 * 480];
    float* pData = indexData;

    for (int i=0; i<640*480; ++i) {
        *(pData++) = i;
    }

    // float distanceData[640 * 480]; // The same as before
    float* distanceData = new float[640 * 480];
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

    delete[] indexData;
    delete[] distanceData;
}

} // End Namespace
