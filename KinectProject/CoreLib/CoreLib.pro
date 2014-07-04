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

# Static Release
#win32 {
#    QMAKE_CFLAGS_RELEASE += /MT
#    QMAKE_CXXFLAGS_RELEASE += /MT
#}

# Use C++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS = -std=c++11

SOURCES += \
    types/Vector3D.cpp \
    types/SkeletonJoint.cpp \
    types/SkeletonFrame.cpp \
    types/Skeleton.cpp \
    types/Quaternion.cpp \
    types/Point3f.cpp \
    types/DepthFrame.cpp \
    types/DataFrame.cpp \
    types/ColorFrame.cpp \
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
    viewer/Model3DItem.cpp \
    viewer/InstanceViewerWindow.cpp \
    viewer/InstanceViewer.cpp \
    viewer/CustomItem.cpp \
    InstanceWidgetItem.cpp \
    types/FrameFilter.cpp \
    filters/InvisibilityFilter.cpp \
    filters/DilateUserFilter.cpp \
    filters/BlurFilter.cpp \
    dataset/MSRDaily/MSRDailyColorInstance.cpp \
    Config.cpp \
    viewer/ViewerEngine.cpp \
    viewer/ViewerRenderer.cpp \
    playback/PlaybackWorker.cpp \
    types/StreamInstance.cpp \
    dataset/DataInstance.cpp \
    types/MaskFrame.cpp \
    playback/NodeProducer.cpp \
    playback/NodeListener.cpp

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
    viewer/QMLEnumsWrapper.h \
    viewer/Model3DItem.h \
    viewer/InstanceViewerWindow.h \
    viewer/InstanceViewer.h \
    viewer/CustomItem.h \
    InstanceWidgetItem.h \
    types/FrameFilter.h \
    filters/InvisibilityFilter.h \
    filters/DilateUserFilter.h \
    filters/BlurFilter.h \
    exceptions/CannotOpenInstanceException.h \
    dataset/MSRDaily/MSRDailyColorInstance.h \
    Config.h \
    viewer/ViewerEngine.h \
    viewer/ViewerRenderer.h \
    playback/PlaybackWorker.h \
    types/MaskFrame.h \
    playback/NodeProducer.h \
    playback/NodeListener.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    #INSTALLS += target
}

unix:!macx {
    # OpenCV2
    # LIBS += -lopencv_core -lopencv_imgproc
    INCLUDEPATH += /usr/include/opencv/
    DEPENDPATH += /usr/include/opencv/
}

win32 {
    INCLUDEPATH += $$PWD
    # OpenCV2
    INCLUDEPATH += "C:/opt/opencv-2.4.9/include"
    DEPENDPATH += "C:/opt/opencv-2.4.9/include"
    contains(QMAKE_TARGET.arch, x86_64):LIBS += -L"C:/opt/opencv-2.4.9/x64/vc11/lib" -lopencv_core249 -lopencv_imgproc249
    contains(QMAKE_TARGET.arch, x86):LIBS += -L"C:/opt/opencv-2.4.9/x86/vc11/lib" -lopencv_core249 -lopencv_imgproc249
}

OTHER_FILES +=

RESOURCES +=
