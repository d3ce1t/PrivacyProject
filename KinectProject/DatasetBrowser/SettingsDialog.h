#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    const QString getMSRDailyActivity3D() const;
    const QString getMSRAction3D() const;
    const QString getHuDaAct() const;
    ~SettingsDialog();

private slots:
    void on_pushButton1_clicked();
    void on_pushButton2_clicked();
    void on_pushButton3_clicked();
    void on_buttonBox_accepted();

private:
    Ui::SettingsDialog *ui;
    QString m_filePath;
    QSettings* m_settings;
};

#endif // SETTINGSWINDOW_H
