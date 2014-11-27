#include "ScenePainter.h"

namespace dai {

QOpenGLFramebufferObject* ScenePainter::createFBO(int width, int height)
{
    QOpenGLFramebufferObject* fbo = nullptr;

    QOpenGLFramebufferObjectFormat format;
    format.setInternalTextureFormat(GL_RGB);
    format.setTextureTarget(GL_TEXTURE_2D);
    format.setSamples(0);
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

    fbo = new QOpenGLFramebufferObject(QSize(width, height), format);

    if (!fbo->isValid()) {
        fbo = nullptr;
        qDebug() << "FBO Error";
        throw 2;
    }

    qDebug() << "handle" << fbo->handle() << "texture" << fbo->texture();

    return fbo;
}

ScenePainter::ScenePainter()
{
    m_scene_width = 640;
    m_scene_height = 480;
    m_initialised = false;
    m_needLoading.store(0);
    m_dirty.store(0);
}

void ScenePainter::clearItems()
{
    m_items.clear();
}

void ScenePainter::markAsDirty()
{
    m_dirty.store(1);
}

bool ScenePainter::isDirty() const
{
    return m_dirty.load() > 0;
}

void ScenePainter::clearDirty()
{
    m_dirty.store(0);
}

void ScenePainter::addItem(shared_ptr<SceneItem> item)
{
    item->m_scene = this;
    item->setMatrix(m_matrix);
    m_items.append(item);
}

shared_ptr<SceneItem> ScenePainter::getFirstItem(ItemType type) const
{
    shared_ptr<SceneItem> result;
    QListIterator<shared_ptr<SceneItem>> it(m_items);

    while (!result && it.hasNext()) {

        shared_ptr<SceneItem> item = it.next();

        if (item->type() == type) {
            result = item;
        }
    }

    return result;
}

void ScenePainter::setBackground(DataFramePtr background)
{
    m_bg = background;
    m_needLoading.store(1);
}

void ScenePainter::resize(int width, int height)
{
    qDebug() << "ScenePainter" << width << height;
    m_scene_width = width;
    m_scene_height = height;
}

void ScenePainter::resetPerspective()
{
    m_matrix.setToIdentity();
    m_matrix.perspective(45, 4/3, 0.1f, 100.0f);
}

int ScenePainter::width() const
{
    return m_scene_width;
}

int ScenePainter::height() const
{
    return m_scene_height;
}

void ScenePainter::setMatrix(const QMatrix4x4 &matrix)
{
    m_matrix = matrix;
}

QMatrix4x4& ScenePainter::getMatrix()
{
    return m_matrix;
}

void ScenePainter::initScene(int width, int height)
{
    if (!m_initialised) {
        m_scene_width = width;
        m_scene_height = height;
        initializeOpenGLFunctions();
        initialise();
        resetPerspective();
        m_initialised = true;
    }
}

void ScenePainter::renderScene(QOpenGLFramebufferObject *target)
{
    qDebug() << "FBO" << target;

    if (!m_initialised) {
        initScene();
    }

    if (isDirty()) {
        render(target);
        clearDirty();
    }
}

void ScenePainter::renderItems()
{
    foreach (shared_ptr<SceneItem> item, m_items)
    {
        item->renderItem();
    }
}

//
// Static Methods
//

// Create Texture (overwrite previous)
void ScenePainter::loadVideoTexture(GLuint glTextureId, GLsizei width, GLsizei height, void* texture)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Create Texture (overwrite previous)
void ScenePainter::loadMaskTexture(GLuint glTextureId, GLsizei width, GLsizei height, void* texture)
{
    glBindTexture(GL_TEXTURE_2D, glTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}


} // End Namespace
