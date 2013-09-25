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

using namespace std;

namespace dai {

class ScenePainter : public QOpenGLFunctions
{
public:
    static void loadVideoTexture(GLuint glTextureId, GLsizei windowWidth, GLsizei windowHeight, void *texture);
    static void loadMaskTexture(GLuint glTextureId, GLsizei windowWidth, GLsizei windowHeight, void *texture);

    ScenePainter();
    virtual ~ScenePainter();
    void clearItems();
    void addItem(shared_ptr<SceneItem> item);
    shared_ptr<SceneItem> getFirstItem(ItemType type) const;
    void setBackground(shared_ptr<DataFrame> background);
    void renderScene();
    void setMatrix(const QMatrix4x4& matrix);
    QMatrix4x4& getMatrix();
    virtual void resetPerspective();
    void setWindowSize(int windowWidth, int windowHeight);
    int windowWidth() const;
    int windowHeight() const;

protected:
    void renderItems();
    virtual void initialise() = 0;
    virtual void render() = 0;

    int                              m_window_width;
    int                              m_window_height;
    shared_ptr<DataFrame>            m_bg;

#if (!defined _MSC_VER || _MSC_VER > 1600)
    atomic<int>                      m_needLoading;
#else
    QAtomicInt                       m_needLoading;
#endif

    QMatrix4x4                       m_matrix;
    QList<shared_ptr<SceneItem>>     m_items;

private: 
    bool                             m_initialised;
};

} // End Namespace

#endif // SCENEPAINTER_H
