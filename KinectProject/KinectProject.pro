QT += qml quick gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KinectProject

# Use C++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS = -std=c++11

HEADERS += \
    dataset/MSRAction3D/MSR3Action3D.h \
    dataset/InstanceInfo.h \
    Grill.h \
    HistogramScene.h \
    dataset/Dataset.h \
    dataset/NotSupportedDatasetException.h \
    MainWindow.h \
    exceptions/NotSupportedDatasetException.h \
    DatasetBrowser.h \
    dataset/DatasetMetadata.h \
    dataset/MSRDaily/MSRDailyActivity3D.h \
    InstanceWidgetItem.h \
    dataset/DataInstance.h \
    viewer/DepthFramePainter.h \
    dataset/MSRDaily/MSRDailyDepthInstance.h \
    viewer/InstanceViewer.h \
    dataset/MSRDaily/MSRDailySkeletonInstance.h \
    types/SkeletonJoint.h \
    types/Skeleton.h \
    types/Point3f.h \
    types/DepthFrame.h \
    types/DataFrame.h \
    types/ColorFrame.h \
    viewer/ColorFramePainter.h \
    dataset/MSRAction3D/MSRActionDepthInstance.h \
    dataset/MSRAction3D/MSRActionSkeletonInstance.h \
    types/Quaternion.h \
    viewer/CustomItem.h \
    types/StreamInstance.h \
    openni/OpenNIDepthInstance.h \
    KMeans.h \
    Utils.h \
    DepthSeg.h \
    dataset/DAI/DAIDataset.h \
    dataset/DAI/DAIDepthInstance.h \
    exceptions/NotImplementedException.h \
    openni/OpenNIColorInstance.h \
    types/GenericFrame.h \
    openni/OpenNIRuntime.h \
    dataset/DAI/DAIColorInstance.h \
    types/Vector3D.h \
    viewer/Painter.h \
    exceptions/NotOpenedInstanceException.h \
    viewer/InstanceViewerWindow.h \
    types/UserFrame.h \
    viewer/InstanceRecorder.h \
    dataset/DAI/DAIUserInstance.h \
    viewer/UserFramePainter.h \
    filters/FrameFilter.h \
    filters/DilateUserFilter.h \
    filters/BlurFilter.h \
    filters/InvisibilityFilter.h \
    viewer/DummyPainter.h \
    viewer/QMLEnumsWrapper.h \
    playback/PlaybackWorker.h \
    playback/PlaybackNotifier.h \
    playback/PlaybackListener.h \
    playback/PlaybackControl.h \
    openni/OpenNIUserInstance.h \
    openni/OpenNISkeletonInstance.h \
    viewer/TestListener.h \
    types/SkeletonFrame.h \
    viewer/SkeletonFramePainter.h \
    dataset/DAI/DAISkeletonInstance.h

SOURCES += \
    dataset/MSRAction3D/MSR3Action3D.cpp \
    dataset/InstanceInfo.cpp \
    Grill.cpp \
    HistogramScene.cpp \
    Main.cpp \
    dataset/Dataset.cpp \
    MainWindow.cpp \
    DatasetBrowser.cpp \
    dataset/DatasetMetadata.cpp \
    dataset/MSRDaily/MSRDailyActivity3D.cpp \
    InstanceWidgetItem.cpp \
    dataset/DataInstance.cpp \
    viewer/DepthFramePainter.cpp \
    dataset/MSRDaily/MSRDailyDepthInstance.cpp \
    viewer/InstanceViewer.cpp \
    dataset/MSRDaily/MSRDailySkeletonInstance.cpp \
    types/SkeletonJoint.cpp \
    types/Skeleton.cpp \
    types/Point3f.cpp \
    types/DepthFrame.cpp \
    types/DataFrame.cpp \
    types/ColorFrame.cpp \
    viewer/ColorFramePainter.cpp \
    dataset/MSRAction3D/MSRActionDepthInstance.cpp \
    dataset/MSRAction3D/MSRActionSkeletonInstance.cpp \
    types/Quaternion.cpp \
    viewer/CustomItem.cpp \
    types/StreamInstance.cpp \
    openni/OpenNIDepthInstance.cpp \
    KMeans.cpp \
    DepthSeg.cpp \
    dataset/DAI/DAIDataset.cpp \
    dataset/DAI/DAIDepthInstance.cpp \
    openni/OpenNIColorInstance.cpp \
    openni/OpenNIRuntime.cpp \
    dataset/DAI/DAIColorInstance.cpp \
    types/Vector3D.cpp \
    viewer/Painter.cpp \
    viewer/InstanceViewerWindow.cpp \
    types/UserFrame.cpp \
    viewer/InstanceRecorder.cpp \
    dataset/DAI/DAIUserInstance.cpp \
    viewer/UserFramePainter.cpp \
    filters/FrameFilter.cpp \
    filters/DilateUserFilter.cpp \
    filters/BlurFilter.cpp \
    filters/InvisibilityFilter.cpp \
    viewer/DummyPainter.cpp \
    playback/PlaybackWorker.cpp \
    playback/PlaybackNotifier.cpp \
    playback/PlaybackListener.cpp \
    playback/PlaybackControl.cpp \
    openni/OpenNIUserInstance.cpp \
    openni/OpenNISkeletonInstance.cpp \
    viewer/TestListener.cpp \
    types/SkeletonFrame.cpp \
    viewer/SkeletonFramePainter.cpp \
    dataset/DAI/DAISkeletonInstance.cpp

RESOURCES += openglunderqml.qrc

FORMS += \
    mainwindow.ui \
    DatasetBrowser.ui


unix:!macx {
    # OpenNI2
    LIBS += -L/opt/OpenNI-Linux-x64-2.2/Tools/ -lOpenNI2
    INCLUDEPATH += /opt/OpenNI-Linux-x64-2.2/Include
    DEPENDPATH += /opt/OpenNI-Linux-x64-2.2/Include

    # NiTE2
    LIBS += -L/opt/NiTE-Linux-x64-2.2/Redist/ -lNiTE2
    INCLUDEPATH += /opt/NiTE-Linux-x64-2.2/Include
    DEPENDPATH += /opt/NiTE-Linux-x64-2.2/Include

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

    # OpenCV2
    LIBS += -lopencv_core -lopencv_imgproc
    INCLUDEPATH += /usr/include/opencv/
    DEPENDPATH += /usr/include/opencv/
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

OTHER_FILES += \
    glsl/textureVertex.vsh \
    glsl/textureFragment.fsh \
    glsl/simpleVertex.vsh \
    glsl/simpleFragment.fsh \
    qml/main.qml \
    glsl/userFragment.fsh \
    glsl/userVertex.vsh \
    glsl/dummyVertex.vsh \
    glsl/dummyFragment.fsh \
    glsl/depthVertex.vsh \
    glsl/depthFragment.fsh
