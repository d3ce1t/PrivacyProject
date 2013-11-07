#ifndef INSTANCE_VIEWER_H
#define INSTANCE_VIEWER_H

#include <QQuickFramebufferObject>
#include "viewer/ViewerEngine.h"

class QListWidget;

class InstanceViewer : public QQuickFramebufferObject
{
    Q_OBJECT

    Q_PROPERTY(ViewerEngine *viewerEngine READ viewerEngine WRITE setViewerEngine)

public:
    explicit InstanceViewer();
    virtual ~InstanceViewer();
    Renderer *createRenderer() const;
    ViewerEngine* viewerEngine() const { return m_viewerEngine; }
    void setViewerEngine(ViewerEngine *viewerEngine);

protected:
    void timerEvent(QTimerEvent *evt);

private:
    ViewerEngine*  m_viewerEngine;
};

#endif // INSTANCE_VIEWER_H
