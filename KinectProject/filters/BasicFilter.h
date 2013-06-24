#ifndef BASICFILTER_H
#define BASICFILTER_H

#include <QObject>
#include "../viewer/InstanceViewer.h"
#include "../types/ColorFrame.h"

namespace dai {

class BasicFilter : public QObject
{
    Q_OBJECT

public:
    BasicFilter();

public slots:
    //void processFrame(dai::DataFrameList framesList, InstanceViewer* viewer);

private:
    void dilateUserMask(short *labels);

    ColorFrame m_background;
};

} // End namespace

#endif // BASICFILTER_H
