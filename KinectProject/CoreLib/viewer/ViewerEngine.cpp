#include "ViewerEngine.h"
#include <QtQml>
#include "playback/PlaybackControl.h"
#include "viewer/InstanceViewer.h"
#include "viewer/SkeletonItem.h"

ViewerEngine::ViewerEngine()
    : m_quickWindow(nullptr)
    , m_running(false)
    , m_initialised(false)
    , m_colorFrame(nullptr)
    , m_metadataFrame(nullptr)
    , m_viewer(nullptr)
    , m_showMode(Disabled)
{
    qmlRegisterType<ViewerEngine>("ViewerEngine", 1, 0, "ViewerEngine");
    qmlRegisterType<InstanceViewer>("InstanceViewer", 1, 0, "InstanceViewer");

    qRegisterMetaType<PlaybackControl*>("PlaybackControl*");
    qRegisterMetaType<const PlaybackControl*>("const PlaybackControl*");
    qRegisterMetaType<QList<shared_ptr<StreamInstance>>>("QList<shared_ptr<StreamInstance>>");
    qRegisterMetaType<shared_ptr<SkeletonFrame>>("shared_ptr<SkeletonFrame>");

    m_needLoading[0] = false;
    m_needLoading[1] = false;

    m_matrix.setToIdentity();
    m_matrix.ortho(0, 640, 480, 0, -1.0, 1.0);
}

ViewerEngine::~ViewerEngine()
{
    m_running = false;
    m_quickWindow = nullptr;
    qDebug() << "ViewerEngine::~ViewerEngine()";
}

void ViewerEngine::setInstanceViewer(InstanceViewer* viewer)
{
    m_viewer = viewer;
}

InstanceViewer* ViewerEngine::viewer()
{
    return m_viewer;
}

void ViewerEngine::setDrawMode(DrawMode mode)
{
    m_showMode = mode;
}

void ViewerEngine::startEngine(QQuickWindow* window)
{
    m_quickWindow = window;
    m_running = true;
}

bool ViewerEngine::running() const
{
    return m_running;
}

// Called from notifier item
void ViewerEngine::prepareScene(dai::QHashDataFrames dataFrames)
{
    if (!m_running)
        return;

    m_dataLock.lock();
    if (dataFrames.contains(DataFrame::Color)) {
        m_colorFrame = static_pointer_cast<ColorFrame>(dataFrames.value(DataFrame::Color));
        m_needLoading[0] = true;
    }

    if (dataFrames.contains(DataFrame::Metadata)) {
        m_metadataFrame = static_pointer_cast<MetadataFrame>(dataFrames.value(DataFrame::Metadata));
        m_needLoading[1] = true;
    }

    m_dataLock.unlock();
}

// Called from rendered item
void ViewerEngine::renderOpenGLScene(QOpenGLFramebufferObject* fbo)
{
    Q_UNUSED(fbo)

    if (!m_running)
        return;

    if (!m_initialised)
        initialise();

    // Init Each Frame (because QtQuick could change it)
    glDepthRange(0.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    // Configure ViewPort and Clear Screen
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glViewport(0, 0, 640, 480);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_perspectiveMatrixUniform, m_matrix);
    m_shaderProgram->setUniformValue(m_noTextureUniform, 0);

    m_dataLock.lock();

    if (m_needLoading[0]) {
        // Load Frame
        glBindTexture(GL_TEXTURE_2D, m_colorTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_colorFrame->getWidth(),  m_colorFrame->getHeight(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, (void *) m_colorFrame->getDataPtr());
        glBindTexture(GL_TEXTURE_2D, 0);
        m_needLoading[0] = false;
    }

    m_dataLock.unlock();

    m_vao.bind();

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_colorTextureId);
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_vao.release();
    m_shaderProgram->release();

    if (m_metadataFrame && m_showMode.testFlag(BoundingBox)) {
        renderBoundingBoxes();
    }

    // Restore
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
}

void ViewerEngine::renderBoundingBoxes()
{
    m_dataLock.lock();

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_noTextureUniform, 1);

    foreach (dai::BoundingBox bb, m_metadataFrame->boundingBoxes())
    {
        // OpenGL coordinates system has y-axis inverted (0 is bottom, instead of top)
        float vertexData[] = {
            bb.getMax().val(0), 480 - bb.getMax().val(1),
            bb.getMax().val(0), 480 - bb.getMin().val(1),
            bb.getMin().val(0), 480 - bb.getMin().val(1),
            bb.getMin().val(0), 480 - bb.getMax().val(1)
        };

        m_shaderProgram->enableAttributeArray(m_posAttr);
        m_shaderProgram->setAttributeArray(m_posAttr, GL_FLOAT, vertexData, 2);

        glDrawArrays(GL_LINE_LOOP, m_posAttr, 4);

        m_shaderProgram->disableAttributeArray(m_posAttr);
    }

    m_shaderProgram->release();

    m_dataLock.unlock();
}

void ViewerEngine::initialise()
{
    initializeOpenGLFunctions();

    // Load, compile and link the shader program
    prepareShaderProgram();

    // Init Vertex Buffer
    prepareVertexBuffer();

    // Create texture
    glGenTextures(1, &m_colorTextureId);
    m_initialised = true;
}

void ViewerEngine::prepareShaderProgram()
{
    m_shaderProgram = new QOpenGLShaderProgram();
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/glsl/glsl/viewer.vsh");
    m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/glsl/glsl/viewer.fsh");
    m_shaderProgram->bindAttributeLocation("posAttr", 0);
    m_shaderProgram->bindAttributeLocation("texCoord", 1);

    m_shaderProgram->link();

    m_posAttr = m_shaderProgram->attributeLocation("posAttr");
    m_textCoordAttr = m_shaderProgram->attributeLocation("texCoord");
    m_perspectiveMatrixUniform = m_shaderProgram->uniformLocation("perspectiveMatrix");
    m_texColorFrameSampler = m_shaderProgram->uniformLocation("texColorFrame");
    m_noTextureUniform = m_shaderProgram->uniformLocation("notexture");

    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue(m_noTextureUniform, 0);
    m_shaderProgram->setUniformValue(m_texColorFrameSampler, 0);
    m_shaderProgram->setUniformValue(m_perspectiveMatrixUniform, m_matrix);
    m_shaderProgram->release();
}

void ViewerEngine::prepareVertexBuffer()
{
    float vertexData[] = {
        0.0, (float) 480,
        (float) 640, (float) 480,
        (float) 640, 0,
        0.0, 0.0
    };

    float texCoordsData[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };

    m_vao.create();
    m_vao.bind();

    m_positionsBuffer.create(); // Create a vertex buffer
    m_positionsBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_positionsBuffer.bind();
    m_positionsBuffer.allocate(vertexData, 4*2*sizeof(float));
    m_shaderProgram->enableAttributeArray(m_posAttr);
    m_shaderProgram->setAttributeBuffer(m_posAttr, GL_FLOAT, 0, 2 );
    m_positionsBuffer.release();

    m_texCoordBuffer.create();
    m_texCoordBuffer.setUsagePattern(QOpenGLBuffer::StreamDraw);
    m_texCoordBuffer.bind();
    m_texCoordBuffer.allocate(texCoordsData, 4*2*sizeof(float));
    m_shaderProgram->enableAttributeArray(m_textCoordAttr);
    m_shaderProgram->setAttributeBuffer(m_textCoordAttr, GL_FLOAT, 0, 2 );
    m_texCoordBuffer.release();

    m_vao.release();
}
