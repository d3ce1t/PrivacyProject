#ifndef INSTANCEWIDGETITEM_H
#define INSTANCEWIDGETITEM_H

#include <QListWidgetItem>
#include "dataset/InstanceInfo.h"

using namespace dai;

class InstanceWidgetItem : public QListWidgetItem
{
public:
    InstanceWidgetItem();
    InstanceWidgetItem(const QString& text, QListWidget* list);
    void setInfo(InstanceInfo& info);
    InstanceInfo& getInfo();

private:
    InstanceInfo m_info;
};

#endif // INSTANCEWIDGETITEM_H
