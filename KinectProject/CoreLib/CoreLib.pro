#-------------------------------------------------
#
# Project created by QtCreator 2013-09-18T13:28:07
#
#-------------------------------------------------

QT       += core qml quick gui multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CoreLib
TEMPLATE = lib
CONFIG += staticlib
CONFIG += create_prl

# Use C++11
unix:!macx {
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_LFLAGS = -std=c++11
}

SOURCES += \
    types/Vector3D.cpp \
    types/SkeletonJoint.cpp \
    types/SkeletonFrame.cpp \
    types/Skeleton.cpp \
    types/Quaternion.cpp \
    types/Point3f.cpp \
    types/DepthFrame.cpp \
    types/DataFrame.cpp \
    dataset/InstanceInfo.cpp \
    dataset/DatasetMetadata.cpp \
    dataset/Dataset.cpp \
    dataset/MSRAction3D/MSRActionSkeletonInstance.cpp \
    dataset/MSRAction3D/MSRActionDepthInstance.cpp \
    dataset/MSRAction3D/MSR3Action3D.cpp \
    dataset/MSRDaily/MSRDailySkeletonInstance.cpp \
    dataset/MSRDaily/MSRDailyDepthInstance.cpp \
    dataset/MSRDaily/MSRDailyActivity3D.cpp \
    playback/PlaybackControl.cpp \
    viewer/SkeletonItem.cpp \
    viewer/SilhouetteItem.cpp \
    viewer/ScenePainter.cpp \
    viewer/SceneItem.cpp \
    viewer/Scene3DPainter.cpp \
    viewer/Scene2DPainter.cpp \
    viewer/InstanceViewerWindow.cpp \
    viewer/InstanceViewer.cpp \
    viewer/CustomItem.cpp \
    InstanceWidgetItem.cpp \
    dataset/MSRDaily/MSRDailyColorInstance.cpp \
    Config.cpp \
    viewer/ViewerEngine.cpp \
    viewer/ViewerRenderer.cpp \
    playback/PlaybackWorker.cpp \
    types/StreamInstance.cpp \
    dataset/DataInstance.cpp \
    playback/FrameGenerator.cpp \
    playback/FrameListener.cpp \
    playback/FrameNotifier.cpp \
    viewer/DepthFilter.cpp \
    types/MetadataFrame.cpp \
    types/BoundingBox.cpp \
    dataset/HuDaAct/HuDaAct.cpp \
    openni/OpenNIColorInstance.cpp \
    openni/OpenNIDepthInstance.cpp \
    openni/OpenNIUserTrackerInstance.cpp \
    openni/OpenNIDevice.cpp \
    viewer/BackgroundItem.cpp

HEADERS += \
    types/Vector3D.h \
    types/StreamInstance.h \
    types/SkeletonJoint.h \
    types/SkeletonFrame.h \
    types/Skeleton.h \
    types/Quaternion.h \
    types/Point3f.h \
    types/GenericFrame.h \
    types/DepthFrame.h \
    types/DataFrame.h \
    types/ColorFrame.h \
    exceptions/NotSupportedDatasetException.h \
    exceptions/NotOpenedInstanceException.h \
    exceptions/NotImplementedException.h \
    dataset/InstanceInfo.h \
    dataset/DatasetMetadata.h \
    dataset/Dataset.h \
    dataset/DataInstance.h \
    dataset/MSRAction3D/MSRActionSkeletonInstance.h \
    dataset/MSRAction3D/MSRActionDepthInstance.h \
    dataset/MSRAction3D/MSR3Action3D.h \
    dataset/MSRDaily/MSRDailySkeletonInstance.h \
    dataset/MSRDaily/MSRDailyDepthInstance.h \
    dataset/MSRDaily/MSRDailyActivity3D.h \
    Utils.h \
    playback/PlaybackControl.h \
    viewer/SkeletonItem.h \
    viewer/SilhouetteItem.h \
    viewer/ScenePainter.h \
    viewer/SceneItem.h \
    viewer/Scene3DPainter.h \
    viewer/Scene2DPainter.h \
    viewer/InstanceViewerWindow.h \
    viewer/InstanceViewer.h \
    viewer/CustomItem.h \
    InstanceWidgetItem.h \
    exceptions/CannotOpenInstanceException.h \
    dataset/MSRDaily/MSRDailyColorInstance.h \
    Config.h \
    viewer/ViewerEngine.h \
    viewer/ViewerRenderer.h \
    playback/PlaybackWorker.h \
    types/MaskFrame.h \
    playback/FrameGenerator.h \
    playback/FrameListener.h \
    playback/FrameNotifier.h \
    viewer/DepthFilter.h \
    viewer/types.h \
    types/MetadataFrame.h \
    types/BoundingBox.h \
    dataset/HuDaAct/HuDaAct.h \
    openni/OpenNIColorInstance.h \
    openni/OpenNIDepthInstance.h \
    openni/OpenNIUserTrackerInstance.h \
    openni/OpenNIDevice.h \
    viewer/BackgroundItem.h

OTHER_FILES +=

RESOURCES += \
    corelib.qrc

unix:!macx {
    # OpenNI2
    LIBS += -L/opt/OpenNI-Linux-x64-2.2/Tools/ -lOpenNI2
    INCLUDEPATH += /opt/OpenNI-Linux-x64-2.2/Include
    DEPENDPATH += /opt/OpenNI-Linux-x64-2.2/Include

    # NiTE2
    LIBS += -L/opt/NiTE-Linux-x64-2.2/Redist/ -lNiTE2
    INCLUDEPATH += /opt/NiTE-Linux-x64-2.2/Include
    DEPENDPATH += /opt/NiTE-Linux-x64-2.2/Include
}

win32 {
    load(moc)
    INCLUDEPATH += $$PWD

    # OpenNI2
    LIBS += -L$$(OPENNI2_LIB) -lOpenNI2
    INCLUDEPATH += $$(OPENNI2_INCLUDE)
    DEPENDPATH += $$(OPENNI2_INCLUDE)

    # NiTE2
    LIBS += -L$$(NITE2_LIB) -lNiTE2
    INCLUDEPATH += $$(NITE2_INCLUDE)
    DEPENDPATH += $$(NITE2_INCLUDE)
}
