#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <QFileDialog>
#include <QSettings>
#include <QDebug>


SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    m_filePath = QApplication::applicationDirPath() + "/settings.ini";
    m_settings = new QSettings(m_filePath, QSettings::IniFormat);

    // load settings
    ui->lineEdit1->setText( m_settings->value("MSRAction3D/path").toString() );
    ui->lineEdit2->setText( m_settings->value("MSRDailyActivity3D/path").toString() );
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
    delete m_settings;
}

const QString SettingsDialog::getMSRDailyActivity3D() const
{
    return m_settings->value("MSRDailyActivity3D/path").toString();
}

const QString SettingsDialog::getMSRAction3D() const
{
    return m_settings->value("MSRAction3D/path").toString();
}

void SettingsDialog::on_pushButton1_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        tr("Select MSR Action3D dataset directory"),
                                                        QDir::currentPath());
    if (!dirName.isEmpty())
        ui->lineEdit1->setText(dirName);
}

void SettingsDialog::on_pushButton2_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this,
                                                        tr("Select MSRDaily Activiy3D dataset directory"),
                                                        QDir::currentPath());
    if (!dirName.isEmpty())
        ui->lineEdit2->setText(dirName);
}

void SettingsDialog::on_buttonBox_accepted()
{
    m_settings->beginGroup("MSRAction3D");
    m_settings->setValue("path", ui->lineEdit1->text());
    m_settings->endGroup();
    m_settings->beginGroup("MSRDailyActivity3D");
    m_settings->setValue("path", ui->lineEdit2->text());
    m_settings->endGroup();
}
