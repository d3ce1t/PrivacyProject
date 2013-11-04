/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#ifndef OGREENGINEITEM_H
#define OGREENGINEITEM_H

#include <QObject>
#include <QQuickWindow>
#include <QOpenGLContext>
#include <Ogre.h>
#include <QElapsedTimer>

namespace Ogre {
class Root;
class RenderTexture;
class Viewport;
class RenderTarget;
}

/**
 * @brief The OgreEngineItem class
 * Must only be constructed from within Qt QML rendering thread.
 */
class OgreEngine : public QObject
{
    Q_OBJECT

public:
    inline static void initResources() { Q_INIT_RESOURCE(ogrelib); }

    OgreEngine();
    ~OgreEngine();
    void startEngine(QQuickWindow *window);
    Ogre::Root* root();
    void stopEngine(Ogre::Root *ogreRoot);
    Ogre::RenderTarget *renderTarget();
    QSGTexture* createTextureFromId(uint id, const QSize &size, QQuickWindow::CreateTextureOptions options = QQuickWindow::CreateTextureOption(0)) const;

public slots:
    void queueRenderingCommand();

signals:
    void beforeRendering(qint64 time_ms);

private:
    void setupResources(void);

    Ogre::String m_resources_cfg;
    Ogre::String m_plugins_cfg;
    Ogre::RenderWindow *m_ogreWindow;
    Ogre::Root* m_root;

    QQuickWindow *m_quickWindow;

    /** Pointer to QOpenGLContext to be used by Ogre. */
    QOpenGLContext* m_ogreContext;
    /** Pointer to QOpenGLContext to be restored after Ogre context. */
    QOpenGLContext* m_qtContext;

    QElapsedTimer   m_timer;
};

#endif // OGREENGINEITEM_H
