#ifndef BASICVIEWER_H
#define BASICVIEWER_H

#include <QMainWindow>
#include <QTimer>
#include "dataset/MSRDailyActivity3DInstance.h"

namespace Ui {
class BasicViewer;
}

class BasicViewer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit BasicViewer(QWidget *parent = 0);
    ~BasicViewer();
    void play(dai::MSRDailyActivity3DInstance* handler);

public slots:
    void readNextFrame();
    
private:
    QTimer* timer;
    Ui::BasicViewer *ui;
    dai::MSRDailyActivity3DInstance* m_handler;
};

#endif // BASICVIEWER_H
