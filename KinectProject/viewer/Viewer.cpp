#include "Viewer.h"
#include <BasicUsageScene.h>
#include <DepthStreamScene.h>
#include <QDebug>
#include <QQmlContext>
#include <QtQml>
#include <iostream>
#include <Grill.h>
#include <HistogramScene.h>
#include "DepthFrame.h"

Viewer::Viewer( QWindow *parent )
    : QQuickView( parent )//, g_poseTimeoutToExit(2000)
{
    m_running = false;
    m_frames = 0;
    m_initialised = false;
    m_lastTime = 0;

    setClearBeforeRendering( false );
    QObject::connect(this, SIGNAL(beforeRendering()), SLOT(renderOpenGLScene()), Qt::DirectConnection);
    QObject::connect(&m_timer, SIGNAL(timeout()), SLOT(update()));

    m_handler = 0;
    m_depthScene = new DepthStreamScene();

    /*m_scene = new BasicUsageScene();
    m_grill = new Grill();
    m_histogram = new HistogramScene;*/

    // QML properties
    //rootContext()->setContextProperty("appSettings1", (QObject *) m_scene);
    rootContext()->setContextProperty("winObject", (QObject *) this);
    rootContext()->setContextProperty("appSettings2", (QObject *) m_depthScene);
    //rootContext()->setContextProperty("skeleton", (QObject *) &m_scene->getSkeleton());
}

Viewer::~Viewer()
{
    if (m_depthScene != NULL) {
        delete m_depthScene;
        m_depthScene = NULL;
    }

   /* if (m_scene != NULL) {
        delete m_scene;
        m_scene = NULL;
    }

    if (m_grill != NULL) {
        delete m_grill;
        m_grill = NULL;
    }

    if (m_histogram != NULL) {
        delete m_histogram;
        m_histogram = NULL;
    }*/

    /*if (m_timer != NULL) {
        delete m_timer;
        m_timer = NULL;
    }*/
}

void Viewer::initialise()
{
    int depth;
    glGetIntegerv(GL_DEPTH_BITS, &depth);

    // Scenes
    m_depthScene->setNativeResolution(320, 240);
    m_depthScene->resize(width(), height());
    m_depthScene->initialise();

    /*m_scene->setNativeResolution(videoMode.getResolutionX(), videoMode.getResolutionY());
    m_scene->resize(width(), height());
    m_scene->initialise();

    m_grill->initialise();

    m_histogram->initialise();
*/

    resetPerspective();
}

void Viewer::play(dai::DataInstance* handler)
{
    m_handler = handler;
    m_handler->open();
    m_time.start();
    m_lastTime = 0;
    m_timer.start( 42 );
    m_running = true;
}

void Viewer::renderOpenGLScene()
{
    // First init (depends of video mode)
    // Make here in order to use the OpenGLContext initalised by QtQuick
    if ( !m_initialised ) {
        initialise();
        m_initialised = true;
    }

    // Init Each Frame (because QtQuick could change it)
    glDepthRange(0.0f, 1.0f);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    // Configure ViewPort and Clear Screen
    glViewport(0, 0, width(), height());
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Prepare Scene
    /*m_depthScene->computeVideoTexture(userTrackerFrame, m_colorFrame);
    m_histogram->setHistogram(m_depthScene->getDepthHistogram());
    const nite::Array<nite::UserData>& users = userTrackerFrame.getUsers();*/

    if (m_running)
    {
        // Compute Frame Per Seconds
        qint64 timeNow = m_time.elapsed();
        m_fps = m_frames / (timeNow / 1000.0f);
        m_frames++;

        // Read each frame one by one
        if (m_handler->hasNext())
        {
            dai::DepthFrame* frame = static_cast<dai::DepthFrame*>(m_handler->nextFrame());
            m_depthScene->computeVideoTexture(*frame);

            m_depthScene->render();





            /*ui->viewer->setDepthMap(*frame);
                ui->viewer->update();
                delete frame;*/
        } else if (m_handler != 0) {
            m_timer.stop();
            m_handler->close();
            m_handler = 0;
            m_running = false;
            qDebug() << "cerrado";
        }

        // Emit signal at 30 Hz
        if ( (timeNow - m_lastTime) >= 400) {
            m_lastTime = timeNow;
            emit changeOfStatus();
        }
    }

    // Restore
    glDisable(GL_DEPTH_TEST);
}

void Viewer::update()
{
    //m_timeMillis = m_time.elapsed();
    //m_scene->update( time );
    QQuickView::update();
}

void Viewer::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed()) {
        /*matrix.ortho(0, width(), height(), 0, -1.0, 1.0);
        m_depthScene->setMatrix(matrix);
        m_scene->setMatrix(matrix);*/
        qDebug() << "New size: " << width() << " Height:" << height();
    }
}

void Viewer::resetPerspective()
{
    matrix.setToIdentity();
    matrix.perspective(70, 1.0, 0.01, 10.0);
    matrix.translate(0, 0, -1.1);
    //m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    //m_histogram->setMatrix(matrix);
    //m_scene->setMatrix(matrix);
}

void Viewer::rotateAxisX(float angle)
{
    matrix.rotate(angle, QVector3D(1, 0, 0));
    //m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    //m_histogram->setMatrix(matrix);
    //m_scene->setMatrix(matrix);
}

void Viewer::rotateAxisY(float angle)
{
    matrix.rotate(angle, QVector3D(0, 1, 0));
    //m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    //m_histogram->setMatrix(matrix);
    //m_scene->setMatrix(matrix);
}

void Viewer::rotateAxisZ(float angle)
{
    matrix.rotate(angle, QVector3D(0, 0, 1));
    //m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    //m_histogram->setMatrix(matrix);
    //m_scene->setMatrix(matrix);
}

void Viewer::translateAxisX(float value)
{
    matrix.translate(value, 0, 0);
    //m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    //m_histogram->setMatrix(matrix);
    //m_scene->setMatrix(matrix);
}

void Viewer::translateAxisY(float value)
{
    matrix.translate(0, value, 0);
    //m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    //m_histogram->setMatrix(matrix);
    //m_scene->setMatrix(matrix);
}

void Viewer::translateAxisZ(float value)
{
    matrix.translate(0, 0, value);
    //m_grill->setMatrix(matrix);
    m_depthScene->setMatrix(matrix);
    //m_histogram->setMatrix(matrix);
    //m_scene->setMatrix(matrix);
}

bool Viewer::event(QEvent* ev)
{
    if (ev->type() == QEvent::Close) {
        this->destroy();
        return true;
    }
    else return QQuickView::event(ev);
}
