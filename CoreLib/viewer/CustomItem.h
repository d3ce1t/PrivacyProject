#ifndef CUSTOMITEM_H
#define CUSTOMITEM_H

#include <QStandardItem>

class CustomItem : public QStandardItem
{
public:
    CustomItem();
    void setNumber(float number);

private:

    float relativeChange(float new_value, float old_value);
    float diff(float new_value, float old_value);

    QBrush m_brush;
    float m_last_diff;
    float m_last_value;
};

#endif // CUSTOMITEM_H
