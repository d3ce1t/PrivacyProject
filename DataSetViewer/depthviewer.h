#ifndef DEPTHVIEWER_H
#define DEPTHVIEWER_H

#include <QWidget>
#include <QBrush>
#include <QPen>
#include <QPixmap>
#include "dataset/Sample.h"

using namespace DataSet;

class DepthViewer : public QWidget
{
    Q_OBJECT
public:
    explicit DepthViewer(QWidget *parent = 0);
    void setDepthMap(const Sample& depthMap);


protected:
    void paintEvent(QPaintEvent *event);

private:
    Sample depthMap;

};

#endif // DEPTHVIEWER_H
