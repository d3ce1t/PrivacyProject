#ifndef SCENEPAINTER_H
#define SCENEPAINTER_H

#include "types/DataFrame.h"
#include "viewer/SceneItem.h"
#include <memory>
#include <atomic>
#include <QList>
#include <QMatrix4x4>
#include <QOpenGLFunctions>

using namespace std;

namespace dai {

class ScenePainter : public QOpenGLFunctions
{
public:
    static void loadVideoTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);
    static void loadMaskTexture(GLuint glTextureId, GLsizei width, GLsizei height, void *texture);

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
    void setSize(int width, int height);
    int width() const;
    int height() const;

protected:
    void renderItems();
    virtual void initialise() = 0;
    virtual void render() = 0;

    int                              m_width;
    int                              m_height;
    shared_ptr<DataFrame>            m_bg;
    atomic<int>                      m_needLoading;
    QMatrix4x4                       m_matrix;
    QList<shared_ptr<SceneItem>>     m_items;

private: 
    bool                             m_initialised;
};

} // End Namespace

#endif // SCENEPAINTER_H
