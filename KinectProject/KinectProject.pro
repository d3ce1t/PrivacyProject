QT += qml quick gui
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
    types/Point3f.h \
    types/DepthFrame.h \
    types/DataFrame.h \
    types/ColorFrame.h \
    viewer/SkeletonPainter.h \
    viewer/ColorFramePainter.h \
    dataset/MSRActionDepthInstance.h \
    dataset/MSRActionSkeletonInstance.h \
    types/SkeletonVector.h \
    types/Quaternion.h \
    viewer/CustomItem.h \
    types/StreamInstance.h \
    openni/OpenNIDepthInstance.h \
    KMeans.h \
    Utils.h \
    DepthSeg.h \
    dataset/DAIDataset.h \
    dataset/DAIDepthInstance.h \
    exceptions/NotImplementedException.h \
    openni/OpenNIColorInstance.h

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
    types/Point3f.cpp \
    types/DepthFrame.cpp \
    types/DataFrame.cpp \
    types/ColorFrame.cpp \
    viewer/SkeletonPainter.cpp \
    viewer/ViewerPainter.cpp \
    viewer/ColorFramePainter.cpp \
    dataset/MSRActionDepthInstance.cpp \
    dataset/MSRActionSkeletonInstance.cpp \
    types/SkeletonVector.cpp \
    types/Quaternion.cpp \
    viewer/CustomItem.cpp \
    types/StreamInstance.cpp \
    openni/OpenNIDepthInstance.cpp \
    KMeans.cpp \
    DepthSeg.cpp \
    dataset/DAIDataset.cpp \
    dataset/DAIDepthInstance.cpp \
    openni/OpenNIColorInstance.cpp

RESOURCES += openglunderqml.qrc

FORMS += \
    mainwindow.ui \
    DatasetBrowser.ui


unix:!macx {
    # OpenNI2
    LIBS += -L/opt/OpenNI-2.2.0-x64/Tools/ -lOpenNI2
    INCLUDEPATH += /opt/OpenNI-2.2.0-x64/Include
    DEPENDPATH += /opt/OpenNI-2.2.0-x64/Include

    # NiTE2
    LIBS += -L/opt/NiTE-2.0.0/Redist/ -lNiTE2
    INCLUDEPATH += /opt/NiTE-2.0.0/Include
    DEPENDPATH += /opt/NiTE-2.0.0/Include

    # VTK
    INCLUDEPATH += /usr/include/vtk-5.8
    DEPENDPATH += /usr/include/vtk-5.8

    # Eigen
    INCLUDEPATH += /usr/include/eigen3/
    DEPENDPATH += /usr/include/eigen3/

    # PCL
    LIBS += -lpcl_common -lpcl_visualization -lboost_system -lpcl_io -lvtkCommon -lvtkFiltering
    INCLUDEPATH += /usr/local/include/pcl-1.7/
    DEPENDPATH += /usr/local/include/pcl-1.7/
}

win32 {
    # OpenNI2
    LIBS += -L"C:/Program Files (x86)/OpenNI2/Lib" -lOpenNI2
    INCLUDEPATH += "C:/Program Files (x86)/OpenNI2/Include"
    DEPENDPATH += "C:/Program Files (x86)/OpenNI2/Include"

    # NiTE2
    LIBS += -L"C:/Program Files (x86)/PrimeSense/NiTE2/Lib" -lNiTE2
    INCLUDEPATH += "C:/Program Files (x86)/PrimeSense/NiTE2/Include"
    DEPENDPATH += "C:/Program Files (x86)/PrimeSense/NiTE2/Include"
}
