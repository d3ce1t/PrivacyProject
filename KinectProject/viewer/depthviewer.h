#ifndef DEPTHVIEWER_H
#define DEPTHVIEWER_H

#include <QWidget>
#include <QBrush>
#include <QPen>
#include <QPixmap>
#include "DepthFrame.h"

class DepthViewer : public QWidget
{
    Q_OBJECT
public:
    explicit DepthViewer(QWidget *parent = 0);
    void setDepthMap(const dai::DepthFrame& depthMap);


protected:
    void paintEvent(QPaintEvent *event);

private:
    dai::DepthFrame depthMap;

};

#endif // DEPTHVIEWER_H
