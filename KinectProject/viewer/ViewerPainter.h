#ifndef ABSTRACTSCENE_H
#define ABSTRACTSCENE_H

#include <QMatrix4x4>
#include "types/DataFrame.h"

namespace dai {

class ViewerPainter
{
public:
    virtual void initialise() = 0;
    virtual void render() = 0;
    virtual void resize( float w, float h ) = 0;
    virtual void setFrame(const dai::DataFrame& frame) = 0;
    virtual void setMatrix(QMatrix4x4& m_matrix);

protected:
    QMatrix4x4              m_matrix;
};

} // End Namespace

#endif // ABSTRACTSCENE_H
