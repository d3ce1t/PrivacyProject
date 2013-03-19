#ifndef WINDOW_H
#define WINDOW_H

#include <QQuickView>
#include <QTime>
#include <QElapsedTimer>

class BasicUsageScene;


class Window : public QQuickView
{
    Q_OBJECT
public:
    explicit Window( QWindow* parent = 0 );

public slots:
    void renderOpenGLScene();
    void update();

protected:
    void resizeEvent(QResizeEvent *event);


private:
   BasicUsageScene* m_scene;
   QElapsedTimer m_time;
};

#endif // WINDOW_H
