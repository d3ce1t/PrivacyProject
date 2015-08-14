!include(../common.pri) {
    error("Couldn't find the common.pri file!")
}

QT += core quick gui concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PersonReid
TEMPLATE = app
CONFIG += console app_bundle link_prl

*-g++ {
    QMAKE_CXXFLAGS += -Wno-unused-local-typedefs
}


HEADERS += \
    PersonReid.h \
    Descriptor.h \
    JointHistograms.h \
    DistancesFeature.h \
    RegionDescriptor.h \
    DescriptorSet.h \
    tests.h

SOURCES += main.cpp \
    PersonReid.cpp \
    Descriptor.cpp \
    DistancesFeature.cpp \
    RegionDescriptor.cpp \
    DescriptorSet.cpp \
    tests.cpp


unix {
    # CoreLib
    LIBS += -L$$BIN_PATH -lCoreLib
    PRE_TARGETDEPS += $$BIN_PATH/libCoreLib.a
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # OpenNI2
    LIBS += -L$$(OPENNI2_REDIST) -lOpenNI2
    INCLUDEPATH += $$(OPENNI2_INCLUDE)
    DEPENDPATH += $$(OPENNI2_INCLUDE)

    # OpenCV2
    #INCLUDEPATH += $$(OPENCV2_INCLUDE)
    #DEPENDPATH += $$(OPENCV2_INCLUDE)
    LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_photo -lopencv_features2d -lopencv_nonfree -lopencv_flann
}

unix:!macx {
    # NiTE2
    #LIBS += -L/opt/NiTE-Linux-x64-2.2/Redist/ -lNiTE2
    INCLUDEPATH += /opt/NiTE-Linux-x64-2.2/Include
    DEPENDPATH += /opt/NiTE-Linux-x64-2.2/Include
}

unix:macx {
    # OpenCV2
    INCLUDEPATH += $$(OPENCV2_INCLUDE)
    DEPENDPATH += $$(OPENCV2_INCLUDE)

    # Boost
    INCLUDEPATH += $$(BOOST_INCLUDE)
    DEPENDPATH += $$(BOOST_INCLUDE)
}

win32 {
    # ensure QMAKE_MOC contains the moc executable path
    load(moc)

    INCLUDEPATH += $$PWD

    # CoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # CoreLib Dynamic
    LIBS += -L$$BIN_PATH/ -lCoreLib

    # CoreLib Static
    PRE_TARGETDEPS += $$BIN_PATH/CoreLib.lib

    # Boost
    BOOSTDIR = $$(BOOST_INCLUDEDIR)
    BOOSTLIB = $$(BOOST_LIBRARYDIR)
    !isEmpty(BOOSTDIR) {
        INCLUDEPATH += $$BOOSTDIR
        win32-g++:CONFIG(release, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-mgw48-mt-1_56 -lboost_thread-mgw48-mt-1_56
        else:win32-g++:CONFIG(debug, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-mgw48-mt-d-1_56 -lboost_thread-mgw48-mt-d-1_56
        else:CONFIG(release, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-vc120-mt-1_56 -lboost_thread-vc120-mt-1_56
        else:CONFIG(debug, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-vc120-mt-gd-1_56 -lboost_thread-vc120-mt-gd-1_56
    }

    # OpenNI2
    #LIBS += -L$$(OPENNI2_LIB) -lOpenNI2
    INCLUDEPATH += $$(OPENNI2_INCLUDE)
    DEPENDPATH += $$(OPENNI2_INCLUDE)

    # NiTE2
    #LIBS += -L$$(NITE2_LIB) -lNiTE2
    INCLUDEPATH += $$(NITE2_INCLUDE)
    DEPENDPATH += $$(NITE2_INCLUDE)

    # OpenCV2
    INCLUDEPATH += $$(OPENCV2_INCLUDE)
    DEPENDPATH += $$(OPENCV2_INCLUDE)
    CONFIG(release, debug|release):LIBS += -L$$(OPENCV2_LIB) -lopencv_core2410 -lopencv_imgproc2410 -lopencv_highgui2410 -lopencv_objdetect2410 -lopencv_photo2410 -lopencv_features2d2410 -lopencv_nonfree2410 -lopencv_flann2410
    else:CONFIG(debug, debug|release):LIBS += -L$$(OPENCV2_LIB) -lopencv_core2410d -lopencv_imgproc2410d -lopencv_highgui2410d -lopencv_objdetect2410d -lopencv_photo2410d -lopencv_features2d2410d -lopencv_nonfree2410d -lopencv_flann2410d
}
