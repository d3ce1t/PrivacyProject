#include "DatasetSelector.h"
#include "ui_DatasetSelector.h"

DatasetSelector::DatasetSelector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatasetSelector)
{
    ui->setupUi(this);
}

DatasetSelector::~DatasetSelector()
{
    delete ui;
}

void DatasetSelector::on_buttonBox_accepted()
{
    this->setResult(ui->comboDataset->currentIndex() + 1);
}
