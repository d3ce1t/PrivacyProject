QT += qml quick
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KinectProject

HEADERS += \
    dataset/MSR3Action3D.h \
    dataset/InstanceInfo.h \
    Grill.h \
    HistogramScene.h \
    dataset/Dataset.h \
    dataset/NotSupportedDatasetException.h \
    MainWindow.h \
    exceptions/NotSupportedDatasetException.h \
    DatasetBrowser.h \
    dataset/DatasetMetadata.h \
    dataset/MSRDailyActivity3D.h \
    InstanceWidgetItem.h \
    dataset/DataInstance.h \
    viewer/DepthFramePainter.h \
    dataset/MSRDailyDepthInstance.h \
    viewer/ViewerPainter.h \
    viewer/InstanceViewer.h \
    dataset/MSRDailySkeletonInstance.h \
    types/SkeletonJoint.h \
    types/Skeleton.h \
    types/Quaternion.h \
    types/Point3f.h \
    types/DepthFrame.h \
    types/DataFrame.h \
    types/ColorFrame.h \
    viewer/SkeletonPainter.h \
    viewer/ColorFramePainter.h \
    dataset/MSRActionDepthInstance.h \
    dataset/MSRActionSkeletonInstance.h

SOURCES += \
    dataset/MSR3Action3D.cpp \
    dataset/InstanceInfo.cpp \
    Grill.cpp \
    HistogramScene.cpp \
    Main.cpp \
    dataset/Dataset.cpp \
    MainWindow.cpp \
    DatasetBrowser.cpp \
    dataset/DatasetMetadata.cpp \
    dataset/MSRDailyActivity3D.cpp \
    InstanceWidgetItem.cpp \
    dataset/DataInstance.cpp \
    viewer/DepthFramePainter.cpp \
    dataset/MSRDailyDepthInstance.cpp \
    viewer/InstanceViewer.cpp \
    dataset/MSRDailySkeletonInstance.cpp \
    types/SkeletonJoint.cpp \
    types/Skeleton.cpp \
    types/Quaternion.cpp \
    types/Point3f.cpp \
    types/DepthFrame.cpp \
    types/DataFrame.cpp \
    types/ColorFrame.cpp \
    viewer/SkeletonPainter.cpp \
    viewer/ViewerPainter.cpp \
    viewer/ColorFramePainter.cpp \
    dataset/MSRActionDepthInstance.cpp \
    dataset/MSRActionSkeletonInstance.cpp

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
    DatasetBrowser.ui
