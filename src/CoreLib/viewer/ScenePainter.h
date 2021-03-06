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

    static QOpenGLFramebufferObject* createFBO(int width, int height);

    ScenePainter();
    virtual ~ScenePainter() {}
    void clearItems();
    void addItem(shared_ptr<SceneItem> item);
    shared_ptr<SceneItem> getFirstItem(ItemType type) const;
    void setBackground(DataFramePtr background);
    void initScene(int width = 640, int height = 480);
    void renderScene(QOpenGLFramebufferObject* target = nullptr);
    void setMatrix(const QMatrix4x4& matrix);
    void markAsDirty();
    void clearDirty();
    QMatrix4x4& getMatrix();
    virtual void resize(int width, int height);
    virtual void resetPerspective();
    int width() const;
    int height() const;

    void loadVideoTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);
    void loadMaskTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);

protected:
    bool isDirty() const;
    virtual void initialise() = 0;
    virtual void render(QOpenGLFramebufferObject* target = nullptr) = 0;
    void renderItems();

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
