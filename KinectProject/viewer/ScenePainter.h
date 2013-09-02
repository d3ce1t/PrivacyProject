#ifndef SCENEPAINTER_H
#define SCENEPAINTER_H

#include "types/DataFrame.h"
#include "viewer/SceneItem.h"
#include <memory>
#include <atomic>
#include <QMap>
#include <QMatrix4x4>
#include <QOpenGLFunctions>

using namespace std;

namespace dai {

class ScenePainter : public QOpenGLFunctions
{
public:
    ScenePainter();
    virtual ~ScenePainter();
    void addItem(shared_ptr<SceneItem> item);
    void setBackground(shared_ptr<DataFrame> background);
    void renderScene();
    void setMatrix(const QMatrix4x4& matrix);
    QMatrix4x4& getMatrix();
    void updateItemsMatrix();
    void resetPerspective();
    void setSize(qreal width, qreal height);

protected:
    virtual void initialise() = 0;
    virtual void render() = 0;

    qreal                            m_width;
    qreal                            m_height;
    shared_ptr<DataFrame>            m_bg;
    atomic<int>                      m_needLoading;
    QMatrix4x4                       m_matrix;

private:
    QMap<int, shared_ptr<SceneItem>> m_items;
    bool                             m_initialised;
};

} // End Namespace

#endif // SCENEPAINTER_H
