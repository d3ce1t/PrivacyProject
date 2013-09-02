#ifndef SCENEITEM_H
#define SCENEITEM_H

namespace dai {

class SceneItem
{
public:
    SceneItem();
    int getZOrder() const;

private:
    int m_z_order; // 0 = first item to be drawn
};

} // End Namespace

#endif // SCENEITEM_H
