QT += qml quick
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KinectProject

HEADERS += \
    AbstractScene.h \
    dataset/MSR3Action3D.h \
    dataset/InstanceInfo.h \
    BasicUsageScene.h \
    DepthStreamScene.h \
    Grill.h \
    HistogramScene.h \
    Skeleton.h \
    dataset/Dataset.h \
    dataset/DatasetEnums.h \
    dataset/NotSupportedDatasetException.h \
    MainWindow.h \
    exceptions/NotSupportedDatasetException.h \
    DatasetBrowser.h \
    dataset/DatasetMetadata.h \
    dataset/MSRDailyActivity3D.h \
    DepthFrame.h \
    dataset/MSRDailyActivity3DInstance.h \
    ColorFrame.h \
    DataFrame.h \
    InstanceWidgetItem.h \
    dataset/DataInstance.h \
    viewer/depthviewer.h \
    viewer/basicviewer.h \
    viewer/Viewer.h

SOURCES += \
    dataset/MSR3Action3D.cpp \
    dataset/InstanceInfo.cpp \
    BasicUsageScene.cpp \
    DepthStreamScene.cpp \
    Grill.cpp \
    HistogramScene.cpp \
    Main.cpp \
    Skeleton.cpp \
    dataset/Dataset.cpp \
    MainWindow.cpp \
    DatasetBrowser.cpp \
    dataset/DatasetMetadata.cpp \
    dataset/MSRDailyActivity3D.cpp \
    DepthFrame.cpp \
    dataset/MSRDailyActivity3DInstance.cpp \
    ColorFrame.cpp \
    DataFrame.cpp \
    InstanceWidgetItem.cpp \
    dataset/DataInstance.cpp \
    viewer/depthviewer.cpp \
    viewer/basicviewer.cpp \
    viewer/Viewer.cpp

RESOURCES += openglunderqml.qrc


# OpenNI2
LIBS += -L/opt/OpenNI-2.1.0-x64/Tools/ -lOpenNI2
INCLUDEPATH += /opt/OpenNI-2.1.0-x64/Include
DEPENDPATH += /opt/OpenNI-2.1.0-x64/Include

# NiTE2
LIBS += -L/opt/NiTE-2.0.0/Redist/ -lNiTE2
INCLUDEPATH += /opt/NiTE-2.0.0/Include
DEPENDPATH += /opt/NiTE-2.0.0/Include

FORMS += \
    mainwindow.ui \
    DatasetBrowser.ui \
    viewer/basicviewer.ui
