#include "InvisibilityFilter.h"

namespace dai {

InvisibilityFilter::InvisibilityFilter()
    : m_background(640, 480)
{
}

void InvisibilityFilter::applyFilter(shared_ptr<DataFrame> frame)
{
    if (m_userMask == nullptr)
        return;

    ColorFrame* colorFrame = (ColorFrame*) frame.get();

    // Get initial background at 20th frame
    if (colorFrame->getIndex() == 20) {
        m_background = *colorFrame;
    }

    // Update Background and make replacement
    for (int i=0; i<m_userMask->getHeight(); ++i)
    {
        for (int j=0; j<m_userMask->getWidth(); ++j)
        {
            uint8_t uLabel = m_userMask->getItem(i, j);

            if (uLabel != 0) {
                if (m_enabled) {
                    RGBColor bgColor = m_background.getItem(i, j);
                    colorFrame->setItem(i, j, bgColor);
                }
            }
            else {
                RGBColor color = colorFrame->getItem(i, j);
                m_background.setItem(i, j, color);
            }
        }
    }
}

} // End namespace