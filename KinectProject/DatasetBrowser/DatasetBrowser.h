#ifndef DATASETBROWSER_H
#define DATASETBROWSER_H

#include <QMainWindow>
#include "SettingsDialog.h"
#include "dataset/Dataset.h"
#include "playback/PlaybackControl.h"

namespace Ui {
    class DatasetBrowser;
}

class QListWidgetItem;

class DatasetBrowser : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit DatasetBrowser(QWidget *parent = 0);
    virtual ~DatasetBrowser();

public slots:
    void openSettings();

private slots:
    void listItemChange(QListWidgetItem * item);
    void instanceItemActivated(QListWidgetItem * item);
    void comboBoxChange(int index);
    void loadDataset();
    void on_btnSelectAllActivities_clicked();
    void on_btnUnselectAllActivities_clicked();
    void on_btnSelectAllActors_clicked();
    void on_btnUnselectAllActors_clicked();
    void on_btnSelectAllSamples_clicked();
    void on_btnUnselectAllSamples_clicked();
    void on_comboDataset_activated(int index);

private:
    void loadInstances();

    Ui::DatasetBrowser*     ui;
    dai::Dataset*           m_dataset;
    dai::PlaybackControl    m_playback;
    SettingsDialog          m_settings;
};

#endif // DATASETBROWSER_H
