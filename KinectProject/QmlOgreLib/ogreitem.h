#ifndef OGREITEM_H
#define OGREITEM_H

#include "ogreengine.h"
#include "ogrecamerawrapper.h"

#include <QQuickFramebufferObject>
#include <QtCore/QPropertyAnimation>

class OgreItem : public QQuickFramebufferObject
{
    Q_OBJECT

    Q_PROPERTY(QObject *camera READ camera WRITE setCamera)
    Q_PROPERTY(OgreEngine *ogreEngine READ ogreEngine WRITE setOgreEngine)

public:
    OgreItem(QQuickItem *parent = 0);

    QObject *camera() const { return qobject_cast<QObject *>(m_camera); }
    void setCamera(QObject *camera);

    OgreEngine *ogreEngine() const { return m_ogreEngineItem; }
    void setOgreEngine(OgreEngine *ogreEngine);

    Renderer *createRenderer() const;

protected:
    void timerEvent(QTimerEvent *evt);

private:
    int m_timerID;
    OgreCameraWrapper *m_camera;
    OgreEngine *m_ogreEngineItem;
};

#endif // OGREITEM_H
