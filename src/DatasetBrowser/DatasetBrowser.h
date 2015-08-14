#ifndef DATASETBROWSER_H
#define DATASETBROWSER_H

#include <QMainWindow>
#include "SettingsDialog.h"
#include "dataset/Dataset.h"
#include "viewer/DepthFilter.h"
#include "playback/PlaybackControl.h"

namespace Ui {
    class DatasetBrowser;
}

class QListWidgetItem;

class DatasetBrowser : public QMainWindow
{
    Q_OBJECT

    Ui::DatasetBrowser*     ui;
    dai::Dataset*           m_dataset;
    dai::PlaybackControl    m_playback;
    dai::DepthFilter        m_depthFilter;
    SettingsDialog          m_settings;
    dai::DataFrame::FrameType m_showType;
    
public:
    explicit DatasetBrowser(QWidget *parent = 0);
    virtual ~DatasetBrowser();

public slots:
    void closeDataset();

private slots:
    void instanceItemActivated(QListWidgetItem * item);
    void on_btnSelectAllActivities_clicked();
    void on_btnUnselectAllActivities_clicked();
    void on_btnSelectAllActors_clicked();
    void on_btnUnselectAllActors_clicked();
    void on_btnSelectAllSamples_clicked();
    void on_btnUnselectAllSamples_clicked();
    void loadInstances();

private:
    void loadDataset(dai::Dataset::DatasetType type);
};

#endif // DATASETBROWSER_H
