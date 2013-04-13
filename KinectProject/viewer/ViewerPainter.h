#ifndef ABSTRACTSCENE_H
#define ABSTRACTSCENE_H

#include <QMatrix4x4>
#include "types/DataFrame.h"
#include "dataset/DataInstance.h"

namespace dai {

class ViewerPainter
{
public:
    explicit ViewerPainter(DataInstance* instance);
    virtual bool prepareNext() = 0;
    virtual void resize( float w, float h ) = 0;
    void setMatrix(QMatrix4x4& m_matrix);
    void renderNow();
    DataInstance& instance() const;

protected:
    virtual void initialise() = 0;
    virtual void render() = 0;

    QMatrix4x4              m_matrix;
    DataInstance*           m_instance;

private:
    bool                    m_initialised;
};

} // End Namespace

#endif // ABSTRACTSCENE_H
