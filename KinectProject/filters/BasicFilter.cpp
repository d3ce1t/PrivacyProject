#include "BasicFilter.h"
#include "../types/DepthFrame.h"
#include <QDebug>

namespace dai {

BasicFilter::BasicFilter()
{
}

void BasicFilter::processFrame(DataFrameList framesList, InstanceViewer* viewer)
{
    Q_UNUSED(viewer);

    dai::ColorFrame* f1 = (dai::ColorFrame*) framesList.at(0);
    dai::DepthFrame* f2 = (dai::DepthFrame*) framesList.at(1);

    if (f1->getIndex() == 20) {
        m_background = *f1;
    }

    for (int i=0; i<f2->getHeight(); ++i)
    {
        for (int j=0; j<f2->getWidth(); ++j)
        {
            short uLabel = f2->getLabel(i, j);

            if (uLabel != 0) {
            //    RGBAColor color = m_background.getItem(i, j);
            //    f1->setItem(i, j, color);
            }
        }
    }
}

} // End namespace
