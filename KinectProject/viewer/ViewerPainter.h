#ifndef ABSTRACTSCENE_H
#define ABSTRACTSCENE_H

#include <QMatrix4x4>
#include "types/DataFrame.h"
#include "types/StreamInstance.h"

class InstanceViewer;

namespace dai {

class ViewerPainter
{
public:
    explicit ViewerPainter(StreamInstance *instance, InstanceViewer* parent = 0);
    virtual ~ViewerPainter();
    virtual bool prepareNext() = 0;
    virtual void resize( float w, float h ) = 0;
    void setMatrix(QMatrix4x4& m_matrix);
    void renderNow();
    StreamInstance& instance() const;

protected:
    virtual void initialise() = 0;
    virtual void render() = 0;

    QMatrix4x4              m_matrix;
    StreamInstance*         m_instance;
    InstanceViewer*         m_viewer;

private:
    bool                    m_initialised;
};

} // End Namespace

#endif // ABSTRACTSCENE_H
