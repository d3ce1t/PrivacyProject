#include "BasicFilter.h"
#include "../types/DepthFrame.h"
#include <QDebug>

namespace dai {

BasicFilter::BasicFilter()
    : m_background(640, 480)
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
            RGBAColor color = f1->getItem(i, j);
            short uLabel = f2->getLabel(i, j);

            if (uLabel == 1) {
                RGBAColor bgColor = m_background.getItem(i, j);
                /*bgColor.red = 1.0;
                bgColor.blue = 0.0;
                bgColor.green = 0.0;*/
                f1->setItem(i, j, bgColor);
            }
            else {
                m_background.setItem(i, j, color);
            }
        }
    }
}

} // End namespace
