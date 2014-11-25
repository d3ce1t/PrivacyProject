!include(../common.pri) {
    error("Couldn't find the common.pri file!")
}

QT  = core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PrivacyFilterLib
TEMPLATE = lib
CONFIG += staticlib create_prl thread

SOURCES += \
    filters/PrivacyFilter.cpp \
    ogre/OgrePointCloud.cpp \
    ogre/OgreScene.cpp \
    ogre/SinbadCharacterController.cpp

HEADERS += \
    filters/PrivacyFilter.h \
    ogre/OgrePointCloud.h \
    ogre/OgreScene.h \
    ogre/SinbadCharacterController.h

unix {
    # CoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib
    LIBS += -L$$OUT_PWD/../CoreLib/ -lCoreLib
    PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/libCoreLib.a

    # Ogre
    CONFIG += link_pkgconfig
    PKGCONFIG += OGRE

    # Boost
    LIBS += -lboost_system

    # OpenCV2
    #INCLUDEPATH += $$(OPENCV2_INCLUDE)
    #DEPENDPATH += $$(OPENCV2_INCLUDE)
    LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_photo -lopencv_features2d -lopencv_nonfree -lopencv_flann
}

win32 {
    # ensure QMAKE_MOC contains the moc executable path
    #load(moc)

    INCLUDEPATH += $$PWD

    # CoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # CoreLib Dynamic
    LIBS += -L$$BIN_PATH -lCoreLib

    # CoreLib Static
    PRE_TARGETDEPS += $$BIN_PATH/CoreLib.lib

    # Ogre
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error(PrivacyFilters needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    } else {
        INCLUDEPATH += $$OGREDIR/include/OGRE
        INCLUDEPATH += $$OGREDIR/include/OGRE/RenderSystems/GL
        CONFIG(release, debug|release):LIBS += -L$$OGREDIR/lib/release -L$$OGREDIR/lib/release/opt -lOgreMain -lRenderSystem_GL
        else:CONFIG(debug, debug|release):LIBS += -L$$OGREDIR/lib/debug -L$$OGREDIR/lib/debug/opt -lOgreMain_d -lRenderSystem_GL_d
    }

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

    # OpenCV2
    INCLUDEPATH += $$(OPENCV2_INCLUDE)
    DEPENDPATH += $$(OPENCV2_INCLUDE)
    CONFIG(release, debug|release):LIBS += -L$$(OPENCV2_LIB) -lopencv_core2410 -lopencv_imgproc2410 -lopencv_highgui2410 -lopencv_objdetect2410 -lopencv_photo2410
    else:CONFIG(debug, debug|release):LIBS += -L$$(OPENCV2_LIB) -lopencv_core2410d -lopencv_imgproc2410d -lopencv_highgui2410d -lopencv_objdetect2410d -lopencv_photo2410d
}

RESOURCES += \
    privacylib.qrc
