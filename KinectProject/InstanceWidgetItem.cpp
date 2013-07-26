#include "InstanceWidgetItem.h"

InstanceWidgetItem::InstanceWidgetItem()
{
}

InstanceWidgetItem::InstanceWidgetItem(const QString& text, QListWidget *list)
    : QListWidgetItem(text, list)
{

}

void InstanceWidgetItem::setInfo(InstanceInfo& info)
{
    m_info = info;
}

InstanceInfo& InstanceWidgetItem::getInfo()
{
    return m_info;
}
