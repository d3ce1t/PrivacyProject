#include "ViewerEngine.h"
#include <QtQml>
#include "playback/PlaybackControl.h"
#include "viewer/InstanceViewer.h"
#include "viewer/SkeletonItem.h"
#include "QMLEnumsWrapper.h"

ViewerEngine::ViewerEngine(ViewerMode mode)
    : QObject()
    , m_quickWindow(nullptr)
    , m_running(false)
    , m_initialised(false)
    , m_needLoading(false)
    , m_colorFrame(nullptr)
    , m_viewer(nullptr)
{
    qmlRegisterType<ViewerEngine>("ViewerEngine", 1, 0, "ViewerEngine");
    qmlRegisterType<InstanceViewer>("InstanceViewer", 1, 0, "InstanceViewer");
    qmlRegisterUncreatableType<QMLEnumsWrapper>("edu.dai.kinect", 1, 0, "ColorFilter", "This exports SomeState enums to QML");

    qRegisterMetaType<QHashDataFrames>("QHashDataFrames");
    qRegisterMetaType<PlaybackControl*>("PlaybackControl*");
    qRegisterMetaType<const PlaybackControl*>("const PlaybackControl*");
    qRegisterMetaType<QList<shared_ptr<StreamInstance>>>("QList<shared_ptr<StreamInstance>>");
    qRegisterMetaType<shared_ptr<SkeletonFrame>>("shared_ptr<SkeletonFrame>");

    // Create Scene
    if (mode == MODE_3D) {
        m_scene = make_shared<Scene3DPainter>();
    }
    else {
        m_matrix.setToIdentity();
        m_matrix.ortho(0, 640, 480, 0, -1.0, 1.0);
    }

    m_mode = mode;
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

void ViewerEngine::startEngine(QQuickWindow* window)
{
    m_quickWindow = window;
    m_running = true;
}

bool ViewerEngine::running() const
{
    return m_running;
}

shared_ptr<dai::ScenePainter> ViewerEngine::scene()
{
    return m_scene;
}

void ViewerEngine::onPlusKeyPressed()
{
    emit plusKeyPressed();
}

void ViewerEngine::onMinusKeyPressed()
{
    emit minusKeyPressed();
}

void ViewerEngine::onSpaceKeyPressed()
{
    emit spaceKeyPressed();
}

void ViewerEngine::prepareScene(dai::QHashDataFrames dataFrames)
{
    if (!m_running)
        return;

    // Background of Scene
    if (m_mode == MODE_2D && dataFrames.contains(DataFrame::Color))
    {
        m_dataLock.lock();
        m_colorFrame = static_pointer_cast<ColorFrame>(dataFrames.value(DataFrame::Color));
        m_needLoading = true;
        m_dataLock.unlock();
    }
    else if (m_mode == MODE_3D && dataFrames.contains(DataFrame::Depth))
    {
        shared_ptr<SkeletonItem> skeletonItem = static_pointer_cast<SkeletonItem>(m_scene->getFirstItem(ITEM_SKELETON));

        // Clear items of the scene
        m_scene->clearItems();

        // Background
        m_scene->setBackground( dataFrames.value(DataFrame::Depth) );

        // Add skeleton item to the scene
        if (dataFrames.contains(DataFrame::Skeleton))
        {
            if (!skeletonItem)
                skeletonItem.reset(new SkeletonItem);

            skeletonItem->setSkeleton( static_pointer_cast<SkeletonFrame>(dataFrames.value(DataFrame::Skeleton)) );
            m_scene->addItem(skeletonItem);
        }

        m_scene->markAsDirty();
    }
}

void ViewerEngine::renderOpenGLScene(QOpenGLFramebufferObject* fbo)
{
    // Draw
    if (m_running)
    {
        if (m_mode == MODE_2D)
        {
            if (!m_initialised)
                initialise();

            render2D(fbo);
        }
        else {
            m_size = fbo->size();
            m_scene->setSize(m_size.width(), m_size.height());
            m_scene->renderScene(fbo);
        }
    }
}

void ViewerEngine::render2D(QOpenGLFramebufferObject* fboDisplay)
{
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

    m_dataLock.lock();

    if (m_needLoading) {
        // Load Frame
        glBindTexture(GL_TEXTURE_2D, m_colorTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_colorFrame->getWidth(),  m_colorFrame->getHeight(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, (void *) m_colorFrame->getDataPtr());
        glBindTexture(GL_TEXTURE_2D, 0);
        m_needLoading = false;
    }

    m_dataLock.unlock();

    if (fboDisplay) {
        fboDisplay->bind();
    } else {
        QOpenGLFramebufferObject::bindDefault();
    }

    m_vao.bind();

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_colorTextureId);
    glDrawArrays(GL_TRIANGLE_FAN, m_posAttr, 4);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_vao.release();
    m_shaderProgram->release();

    // Restore
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
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

    m_shaderProgram->bind();
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

void ViewerEngine::enableFilter(int filter)
{
    /*if (m_mode == MODE_2D) {
        shared_ptr<Scene2DPainter> scene = static_pointer_cast<Scene2DPainter>(m_scene);
        scene->enableFilter( (QMLEnumsWrapper::ColorFilter) filter);
    }*/
}

void ViewerEngine::resetPerspective()
{
    m_scene->resetPerspective();
}

void ViewerEngine::rotateAxisX(float angle)
{
    m_scene->getMatrix().rotate(angle, QVector3D(1, 0, 0));
}

void ViewerEngine::rotateAxisY(float angle)
{
    m_scene->getMatrix().rotate(angle, QVector3D(0, 1, 0));
}

void ViewerEngine::rotateAxisZ(float angle)
{
    m_scene->getMatrix().rotate(angle, QVector3D(0, 0, 1));
}

void ViewerEngine::translateAxisX(float value)
{
    m_scene->getMatrix().translate(value, 0, 0);
}

void ViewerEngine::translateAxisY(float value)
{
    m_scene->getMatrix().translate(0, value, 0);
}

void ViewerEngine::translateAxisZ(float value)
{
    m_scene->getMatrix().translate(0, 0, value);
}
