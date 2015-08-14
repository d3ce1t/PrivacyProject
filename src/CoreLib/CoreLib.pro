!include(../common.pri) {
    error("Couldn't find the common.pri file!")
}

QT       += core qml quick gui multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CoreLib
TEMPLATE = lib
CONFIG += staticlib create_prl

SOURCES += \
    types/SkeletonJoint.cpp \
    types/SkeletonFrame.cpp \
    types/Skeleton.cpp \
    types/Quaternion.cpp \
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
    viewer/BackgroundItem.cpp \
    dataset/CAVIAR4REID/CAVIAR4REID.cpp \
    dataset/CAVIAR4REID/CAVIAR4REIDInstance.cpp \
    dataset/DAI4REID/DAI4REID.cpp \
    dataset/DAI4REID_Parsed/DAI4REID_Parsed.cpp \
    dataset/DAI4REID_Parsed/DAI4REID_ParsedInstance.cpp \
    dataset/IASLAB_RGBD_ID/IASLAB_RGBD_ID.cpp \
    dataset/IASLAB_RGBD_ID/IASLAB_RGBD_ID_Instance.cpp

HEADERS += \
    types/Vector3D.h \
    types/StreamInstance.h \
    types/SkeletonJoint.h \
    types/SkeletonFrame.h \
    types/Skeleton.h \
    types/Quaternion.h \
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
    viewer/BackgroundItem.h \
    types/Histogram.h \
    types/Point.h \
    ml/KMeans.h \
    dataset/CAVIAR4REID/CAVIAR4REID.h \
    dataset/CAVIAR4REID/CAVIAR4REIDInstance.h \
    opencv_utils.h \
    types/Enums.h \
    dataset/DAI4REID/DAI4REID.h \
    dataset/DAI4REID_Parsed/DAI4REID_Parsed.h \
    dataset/DAI4REID_Parsed/DAI4REID_ParsedInstance.h \
    dataset/IASLAB_RGBD_ID/IASLAB_RGBD_ID_Instance.h \
    dataset/IASLAB_RGBD_ID/IASLAB_RGBD_ID.h

RESOURCES += \
    corelib.qrc

unix {
    # OpenNI2
    LIBS += -L/$$(OPENNI2_REDIST) -lOpenNI2
    INCLUDEPATH += $$(OPENNI2_INCLUDE)
    DEPENDPATH += $$(OPENNI2_INCLUDE)
}

unix:!macx {
    # OpenCV2
    LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_photo

    # NiTE2
    LIBS += -L/opt/NiTE-Linux-x64-2.2/Redist/ -lNiTE2
    INCLUDEPATH += /opt/NiTE-Linux-x64-2.2/Include
    DEPENDPATH += /opt/NiTE-Linux-x64-2.2/Include
}

unix:macx {
    #GLM
    INCLUDEPATH += $$(GLM_INCLUDE)
    DEPENDPATH += $$(GLM_INCLUDE)

    # OpenCV2
    INCLUDEPATH += $$(OPENCV2_INCLUDE)
    DEPENDPATH += $$(OPENCV2_INCLUDE)
    LIBS += -L/usr/local/lib/ -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_photo
}

win32 {
    #load(moc)
    INCLUDEPATH += $$PWD

    # GLM
    INCLUDEPATH += $$(GLM_INCLUDE)
    DEPENDPATH += $$(GLM_INCLUDE)

    # Boost
    BOOSTDIR = $$(BOOST_INCLUDEDIR)
    #BOOSTLIB = $$(BOOST_LIBRARYDIR)
    !isEmpty(BOOSTDIR) {
        INCLUDEPATH += $$BOOSTDIR
        #win32-g++:CONFIG(release, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-mgw48-mt-1_55 -lboost_thread-mgw48-mt-1_55
        #else:win32-g++:CONFIG(debug, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-mgw48-mt-d-1_55 -lboost_thread-mgw48-mt-d-1_55
        #else:CONFIG(release, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-vc120-mt-1_55 -lboost_thread-vc120-mt-1_55
        #else:CONFIG(debug, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-vc120-mt-gd-1_55 -lboost_thread-vc120-mt-gd-1_55
    }

    # OpenNI2
    LIBS += -L$$(OPENNI2_LIB) -lOpenNI2
    INCLUDEPATH += $$(OPENNI2_INCLUDE)
    DEPENDPATH += $$(OPENNI2_INCLUDE)

    # NiTE2
    LIBS += -L$$(NITE2_LIB) -lNiTE2
    INCLUDEPATH += $$(NITE2_INCLUDE)
    DEPENDPATH += $$(NITE2_INCLUDE)

    # OpenCV2
    INCLUDEPATH += $$(OPENCV2_INCLUDE)
    DEPENDPATH += $$(OPENCV2_INCLUDE)
    CONFIG(release, debug|release):LIBS += -L$$(OPENCV2_LIB) -lopencv_core2410 -lopencv_imgproc2410 -lopencv_highgui2410 -lopencv_objdetect2410 -lopencv_photo2410
    else:CONFIG(debug, debug|release):LIBS += -L$$(OPENCV2_LIB) -lopencv_core2410d -lopencv_imgproc2410d -lopencv_highgui2410d -lopencv_objdetect2410d -lopencv_photo2410d
}
