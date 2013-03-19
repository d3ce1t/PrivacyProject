#include "window.h"
#include <basicusagescene.h>
#include <QTimer>
#include <QDebug>

Window::Window( QWindow *parent )
    : QQuickView( parent ),
      m_scene( new BasicUsageScene )
{
    setClearBeforeRendering( false );

    QObject::connect(this, SIGNAL(beforeRendering()), SLOT(renderOpenGLScene()), Qt::DirectConnection);

    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(update()));
    timer->start( 16 );

    QSurfaceFormat format;
    format.setMajorVersion(2);
    format.setMinorVersion(0);
    format.setSamples(4);
    format.setRenderableType(QSurfaceFormat::OpenGLES);
    format.setProfile( QSurfaceFormat::CoreProfile);
    setFormat( format );
}

void Window::renderOpenGLScene()
{
    static bool firstTime = true;

    if ( firstTime )
    {
        m_scene->initialise();
        m_time.start();
        qDebug() << "Width: " << width() << " Height: " << height();
        firstTime = false;
    }

    //qDebug() << "Rendering";
    m_scene->render();
}

void Window::update()
{
    float time = m_time.elapsed() / 1000.0f;
    m_scene->update( time );
    QQuickView::update();
}

void Window::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed()) {
        m_scene->resize(width(), height());
        qDebug() << "New size: " << width() << " Height:" << height();
    }
}
