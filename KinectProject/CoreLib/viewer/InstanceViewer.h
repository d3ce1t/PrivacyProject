#ifndef INSTANCE_VIEWER_H
#define INSTANCE_VIEWER_H

#include <QQuickItem>
#include <QMatrix4x4>
#include <QMutex>
#include <QHash>
#include <QList>
#include <QListWidget>
#include "types/DataFrame.h"
#include "types/UserFrame.h"
#include "types/FrameFilter.h"
#include "viewer/ScenePainter.h"
#include "viewer/QMLEnumsWrapper.h"
#include "viewer/ViewerEngine.h"

class QListWidget;

class InstanceViewer : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(ViewerEngine *viewerEngine READ viewerEngine WRITE setViewerEngine)

public:
    explicit InstanceViewer();
    virtual ~InstanceViewer();    
    ViewerEngine* viewerEngine() const { return m_viewerEngine; }
    void setViewerEngine(ViewerEngine *viewerEngine);

public slots:
    void renderOpenGLScene();

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:
    ViewerEngine*  m_viewerEngine;
    QQuickWindow*  m_quickWindow;
};

#endif // INSTANCE_VIEWER_H
