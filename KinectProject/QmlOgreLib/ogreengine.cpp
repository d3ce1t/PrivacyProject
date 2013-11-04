/*!
 * \copyright (c) Nokia Corporation and/or its subsidiary(-ies) (qt-info@nokia.com) and/or contributors
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 *
 * \license{This source file is part of QmlOgre abd subject to the BSD license that is bundled
 * with this source code in the file LICENSE.}
 */

#include "ogreengine.h"
#include "ogreitem.h"

#include <QOpenGLFunctions>

OgreEngine::OgreEngine()
    : QObject()
    , m_resources_cfg("resources.cfg")
    , m_plugins_cfg("plugins.cfg")
{
    qmlRegisterType<OgreItem>("Ogre", 1, 0, "OgreItem");
    qmlRegisterType<OgreEngine>("OgreEngine", 1, 0, "OgreEngine");
}

OgreEngine::~OgreEngine()
{
    delete m_ogreContext;
}

Ogre::Root* OgreEngine::root()
{
    return m_root;
}

void OgreEngine::startEngine(QQuickWindow *window)
{
    m_quickWindow = window;
    m_root = new Ogre::Root(m_plugins_cfg);

    // Set OpenGL render subsystem
    Ogre::RenderSystem *renderSystem = m_root->getRenderSystemByName("OpenGL Rendering Subsystem");
    m_root->setRenderSystem(renderSystem);
    m_root->initialise(false);

    // Setup window params
    Ogre::NameValuePairList params;
    params["externalGLControl"] = "true";
    params["currentGLContext"] = "true";

    //Finally create our window.
    m_ogreWindow = m_root->createRenderWindow("OgreWindow", 1, 1, false, &params);
    m_ogreWindow->setVisible(false);
    m_ogreWindow->update(false);

    setupResources();

    m_timer.start();
}

Ogre::RenderTarget* OgreEngine::renderTarget()
{
    return m_ogreWindow;
}

void OgreEngine::stopEngine(Ogre::Root *ogreRoot)
{
    if (ogreRoot) {
        //m_root->detachRenderTarget(m_renderTexture);
        // TODO tell node(s) to detach
    }

    delete ogreRoot;
}

QSGTexture* OgreEngine::createTextureFromId(uint id, const QSize &size, QQuickWindow::CreateTextureOptions options) const
{
    return m_quickWindow->createTextureFromId(id, size, options);
}

void OgreEngine::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(m_resources_cfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;

            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }

    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void OgreEngine::queueRenderingCommand()
{
   emit beforeRendering(m_timer.elapsed());
}
