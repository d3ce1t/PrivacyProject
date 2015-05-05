#include <QApplication>
#include "Config.h"
#include "MainWindow.h"
#include "filters/PrivacyFilter.h"
#include <opencv2/opencv.hpp>


int main(int argc, char *argv[])
{
    CoreLib_InitResources();
    PrivacyLib_InitResources();
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
    /*cv::Mat im = cv::imread("/home/jpadilla/flower.png");
    cout << im.rows << " " << im.cols << std::endl;

    for (int i=0; i<im.rows; i+=5) {

        for (int j=0; j<im.cols; j+=5) {

            cv::Mat block = im.colRange(j, j+5).rowRange(i, i+5);
            cv::Vec3i color;

            for (int k=0; k<5; ++k) { // rows
                color += block.at<cv::Vec3b>(k, 0);
                color += block.at<cv::Vec3b>(k, 1);
                color += block.at<cv::Vec3b>(k, 2);
                color += block.at<cv::Vec3b>(k, 3);
                color += block.at<cv::Vec3b>(k, 4);
            }

            color /= 25;
            block.setTo(color);
        }
    }

    cv::imwrite("/home/jpadilla/out.png", im);*/

}
