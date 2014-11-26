#ifndef DATASETSELECTOR_H
#define DATASETSELECTOR_H

#include <QDialog>

namespace Ui {
class DatasetSelector;
}

class DatasetSelector : public QDialog
{
    Q_OBJECT

public:
    explicit DatasetSelector(QWidget *parent = 0);
    ~DatasetSelector();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::DatasetSelector *ui;
};

#endif // DATASETSELECTOR_H
