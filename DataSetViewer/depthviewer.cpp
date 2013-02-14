#include "depthviewer.h"
#include <QtGui>
#include <iostream>
#include <QColor>
#include <QRgb>
#include <QThread>

using namespace std;

DepthViewer::DepthViewer(QWidget *parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void DepthViewer::paintEvent(QPaintEvent * /* event */)
{
    if (depthMap.GetNCols() != 0 && depthMap.GetNRows() != 0) {

        QPainter painter(this);
        painter.setPen(palette().dark().color());
        painter.setBrush(Qt::NoBrush);

        int minValue = depthMap.MinValue();
        int maxValue = depthMap.MaxValue();

        for (int i=0; i<depthMap.GetNRows(); ++i) {
            for (int j=0; j<depthMap.GetNCols(); ++j) {

                float distance = (float) depthMap.GetItem(i, j);
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

void DepthViewer::setDepthMap(const Sample& depthMap)
{
    this->depthMap = depthMap;
}
