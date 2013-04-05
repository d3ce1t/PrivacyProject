#ifndef INSTANCEWIDGETITEM_H
#define INSTANCEWIDGETITEM_H

#include <QListWidgetItem>
#include "dataset/InstanceInfo.h"

namespace dai {

class InstanceWidgetItem : public QListWidgetItem
{
public:
    InstanceWidgetItem();
    InstanceWidgetItem(const QString& text, QListWidget*& list);
    void setInfo(InstanceInfo& info);
    InstanceInfo& getInfo();

private:
    InstanceInfo m_info;
};

} // End Namespace

#endif // INSTANCEWIDGETITEM_H
