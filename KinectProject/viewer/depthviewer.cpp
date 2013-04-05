#include "depthviewer.h"
#include <QtGui>
#include <iostream>
#include <QColor>
#include <QRgb>
#include <QThread>

using namespace std;

DepthViewer::DepthViewer(QWidget *parent)
    : QWidget(parent), depthMap(1, 1)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void DepthViewer::paintEvent(QPaintEvent * /* event */)
{
    if (depthMap.getWidth() != 0 && depthMap.getHeight() != 0) {

        QPainter painter(this);
        painter.setPen(palette().dark().color());
        painter.setBrush(Qt::NoBrush);

        int minValue = depthMap.minValue();
        int maxValue = depthMap.maxValue();

        for (int i=0; i<depthMap.getHeight(); ++i) {
            for (int j=0; j<depthMap.getWidth(); ++j) {

                float distance = (float) depthMap.getItem(i, j);
                float mycolor = 0;

                if (distance > 0) {
                    mycolor = (distance - minValue) / (maxValue - minValue);
                }


                //cerr << minValue << "-" << maxValue << " -> " << distance << "#" << mycolor << endl;

                QColor color = QColor::fromRgbF(mycolor, mycolor, mycolor);

                painter.setPen(color);
                painter.drawPoint(j, i);
            }
        }
    }
}

void DepthViewer::setDepthMap(const dai::DepthFrame &depthMap)
{
    this->depthMap = depthMap;
}
