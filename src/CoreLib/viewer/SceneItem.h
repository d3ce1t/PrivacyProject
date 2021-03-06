#ifndef SCENEITEM_H
#define SCENEITEM_H

#include <QOpenGLFunctions>
#include <QMatrix4x4>

namespace dai {

enum ItemType {
    ITEM_SILHOUETTE,
    ITEM_SKELETON,
    ITEM_3DMODEL,
    ITEM_BACKGROUND
};

class ScenePainter;

class SceneItem : public QOpenGLFunctions
{
    friend class ScenePainter;

public:
    explicit SceneItem(ItemType type);
    int getZOrder() const;
    void initItem();
    void renderItem(int pass = 1);
    ScenePainter *scene() const;
    void setBackgroundTex(GLuint id);
    void setMatrix(const QMatrix4x4& matrix);
    int neededPasses() const;
    ItemType type() const;
    bool isVisible() const;
    void setVisible(bool value);

protected:
    virtual void initialise() = 0;
    virtual void render(int pass) = 0;

    int m_neededPasses;
    ScenePainter* m_scene;
    QMatrix4x4 m_matrix;

    // OpenGL identifiers
    GLuint m_fgTextureId;

private:
    ItemType m_type;
    int m_z_order; // 0 = first item to be drawn
    bool m_initialised;
    bool m_visible;
};

} // End Namespace

#endif // SCENEITEM_H
