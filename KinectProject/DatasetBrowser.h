#ifndef DATASETBROWSER_H
#define DATASETBROWSER_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "dataset/Dataset.h"
#include "viewer/InstanceViewer.h"


namespace Ui {
    class DatasetBrowser;
}

class DatasetBrowser : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit DatasetBrowser(QWidget *parent = 0);
    virtual ~DatasetBrowser();

protected:
    void closeEvent(QCloseEvent * event);

private slots:
    void listItemChange(QListWidgetItem * item);
    void instanceItemActivated(QListWidgetItem * item);
    void comboBoxChange(int index);
    void on_btnSelectAllActivities_clicked();
    void on_btnUnselectAllActivities_clicked();
    void on_btnSelectAllActors_clicked();
    void on_btnUnselectAllActors_clicked();
    void on_btnSelectAllSamples_clicked();
    void on_btnUnselectAllSamples_clicked();
    void viewerClosed(InstanceViewer *viewer);
    void on_comboDataset_activated(int index);

private:
    void loadDataset(dai::Dataset::DatasetType type);
    void loadInstances();

    Ui::DatasetBrowser*     ui;
    dai::Dataset*           m_dataset;
    dai::PlaybackControl    m_playback;
};

#endif // DATASETBROWSER_H
