#ifndef SCENEPAINTER_H
#define SCENEPAINTER_H

#include "types/DataFrame.h"
#include "viewer/SceneItem.h"
#include <memory>

// Use C++11 atomic if compiler is superior to MSVC 10.0
#if (!defined _MSC_VER || _MSC_VER > 1600)
    #include <atomic>
#else
    #include <QAtomicInt>
#endif


#include <QList>
#include <QMatrix4x4>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>

using namespace std;

namespace dai {

class ScenePainter : public QOpenGLFunctions
{
public:
    ScenePainter();
    virtual ~ScenePainter();
    void clearItems();
    void addItem(shared_ptr<SceneItem> item);
    shared_ptr<SceneItem> getFirstItem(ItemType type) const;
    void setBackground(shared_ptr<DataFrame> background);
    void renderScene(QOpenGLFramebufferObject* fbo);
    void setMatrix(const QMatrix4x4& matrix);
    void markAsDirty();
    bool isDirty() const;
    void clearDirty();
    QMatrix4x4& getMatrix();
    virtual void resetPerspective();
    void setSize(int width, int height);
    int width() const;
    int height() const;

    void loadVideoTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);
    void loadMaskTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);

protected:
    void renderItems();
    virtual void initialise() = 0;
    virtual void render(QOpenGLFramebufferObject* fboDisplay) = 0;

    int                              m_scene_width;
    int                              m_scene_height;
    shared_ptr<DataFrame>            m_bg;
    atomic<int>                      m_needLoading;
    QMatrix4x4                       m_matrix;
    QList<shared_ptr<SceneItem>>     m_items;

private: 
    bool                             m_initialised;
    atomic<int>                      m_dirty;

};

} // End Namespace

#endif // SCENEPAINTER_H
