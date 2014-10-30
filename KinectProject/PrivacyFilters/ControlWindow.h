#ifndef CONTROLWINDOW_H
#define CONTROLWINDOW_H

#include <QMainWindow>

namespace Ui {
class ControlWindow;
}

namespace dai {
class PrivacyFilter;
}

class ControlWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ControlWindow(dai::PrivacyFilter* privacy, QWidget *parent = 0);
    ~ControlWindow();

private slots:
    void on_btnNoPrivacy_clicked();
    void on_btnBlurring_clicked();
    void on_btnPixelating_clicked();
    void on_btnEmboss_clicked();
    void on_btnSilhouette_clicked();
    void on_btnSkeleton_clicked();
    void on_btnAvatar_clicked();
    void on_btnInvisibility_clicked();

    void on_btnSaveImage_clicked();

private:
    Ui::ControlWindow *ui;
    dai::PrivacyFilter *m_privacy;
};

#endif // CONTROLWINDOW_H
