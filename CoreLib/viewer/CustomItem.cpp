#include "CustomItem.h"
#include <cmath>
#include <QDebug>

CustomItem::CustomItem()
{
    m_brush.setStyle(Qt::SolidPattern);
    m_last_diff = 0;
}

void CustomItem::setNumber(float number)
{
    /*float diff = CustomItem::diff(number, m_last_value);
    float diff_factor = diff / m_last_diff;

    m_last_value = number;
    m_last_diff = diff;

    // Set color
    QColor color(255, 255, 255);

    if (diff_factor > 2)
        color.setRgb(255, 0, 0);

*/
    /*if (factor > 0.5)
        factor = 0.5;

    color.setAlphaF( pow(1.5 + 1.5*factor, 8) / 657 );*/
    /*m_brush.setColor(color);
    QStandardItem::setBackground(m_brush);*/

    // Set Text
    QStandardItem::setText(QString::number(number));
}

float CustomItem::diff(float new_value, float old_value)
{
    return fabs(new_value - old_value);
}

float CustomItem::relativeChange(float new_value, float old_value)
{
    float result = 0;

    if (new_value < old_value && old_value != 0) {
        result = 1 - (new_value / old_value);
    }
    else if (new_value != 0) {
        result = 1 - (old_value / new_value);
    }

    return fabs(result);
}
