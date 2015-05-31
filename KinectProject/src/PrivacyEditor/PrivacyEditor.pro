!include(../common.pri) {
    error("Couldn't find the common.pri file!")
}

QT += core gui quick
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PrivacyEditor
TEMPLATE = app

SOURCES += main.cpp\
        MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

RESOURCES += \
    resources.qrc

unix {
    # CoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib
    LIBS += -L$$BIN_PATH -lCoreLib
    PRE_TARGETDEPS += $$BIN_PATH/libCoreLib.a

    # PrivacyFiltersLib
    INCLUDEPATH += $$PWD/../PrivacyFilterLib
    DEPENDPATH += $$PWD/../PrivacyFilterLib
    LIBS += -L$$BIN_PATH -lPrivacyFilterLib
    PRE_TARGETDEPS += $$BIN_PATH/libPrivacyFilterLib.a    
}

unix:!macx {
    # Ogre
    CONFIG += link_pkgconfig
    PKGCONFIG += OGRE

    # Boost
    LIBS += -lboost_system
}

unix:macx {
    # Ogre
    INCLUDEPATH += $$(OGRE_INCLUDE)
    DEPENDPATH += $$(OGRE_INCLUDE)
    LIBS += -F$$(OGRE_LIB)/macosx/Release -framework Ogre

    # Boost
    INCLUDEPATH += $$(BOOST_INCLUDE)
    DEPENDPATH += $$(BOOST_INCLUDE)
    LIBS += -L$$(BOOST_LIB) -lboost_system

    # OpenCV2
    INCLUDEPATH += $$(OPENCV2_INCLUDE)
    DEPENDPATH += $$(OPENCV2_INCLUDE)
}

win32 {
    # ensure QMAKE_MOC contains the moc executable path
    load(moc)

    INCLUDEPATH += $$PWD

    # CoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # CoreLib Dynamic
    LIBS += -L$$BIN_PATH -lCoreLib

    # CoreLib Static
    PRE_TARGETDEPS += $$BIN_PATH/CoreLib.lib

    # PrivacyFiltersLib
    INCLUDEPATH += $$PWD/../PrivacyFilterLib
    DEPENDPATH += $$PWD/../PrivacyFilterLib

    # PrivacyFiltersLib Dynamic
    LIBS += -L$$BIN_PATH -lPrivacyFilterLib

    # PrivacyFiltersLib Static
    PRE_TARGETDEPS += $$BIN_PATH/PrivacyFilterLib.lib

    # Ogre
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error(PrivacyFilters needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    } else {
        INCLUDEPATH += $$OGREDIR/include/OGRE
        INCLUDEPATH += $$OGREDIR/include/OGRE/RenderSystems/GL
    #    CONFIG(release, debug|release):LIBS += -L$$OGREDIR/lib/Release -L$$OGREDIR/lib/Release/opt -lOgreMain -lRenderSystem_GL
    #    else:CONFIG(debug, debug|release):LIBS += -L$$OGREDIR/lib/Rebug -L$$OGREDIR/lib/Debug/opt -lOgreMain_d -lRenderSystem_GL_d
    }

    # Boost
    BOOSTDIR = $$(BOOST_INCLUDEDIR)
    BOOSTLIB = $$(BOOST_LIBRARYDIR)
    !isEmpty(BOOSTDIR) {
        INCLUDEPATH += $$BOOSTDIR
    #   CONFIG(release, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-vc120-mt-1_56 -lboost_thread-vc120-mt-1_56
    #   else:CONFIG(debug, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-vc120-mt-gd-1_56 -lboost_thread-vc120-mt-gd-1_56
    }

    # OpenNI2
    #LIBS += -L$$(OPENNI2_LIB) -lOpenNI2
    #INCLUDEPATH += $$(OPENNI2_INCLUDE)
    #DEPENDPATH += $$(OPENNI2_INCLUDE)

    # NiTE2
    #LIBS += -L$$(NITE2_LIB) -lNiTE2
    #INCLUDEPATH += $$(NITE2_INCLUDE)
    #DEPENDPATH += $$(NITE2_INCLUDE)

    # OpenCV2
    INCLUDEPATH += $$(OPENCV2_INCLUDE)
    DEPENDPATH += $$(OPENCV2_INCLUDE)
    CONFIG(release, debug|release):LIBS += -L$$(OPENCV2_LIB) -lopencv_core2410 -lopencv_imgproc2410 -lopencv_highgui2410 -lopencv_objdetect2410 -lopencv_photo2410 -lopencv_features2d2410 -lopencv_nonfree2410 -lopencv_flann2410
    else:CONFIG(debug, debug|release):LIBS += -L$$(OPENCV2_LIB) -lopencv_core2410d -lopencv_imgproc2410d -lopencv_highgui2410d -lopencv_objdetect2410d -lopencv_photo2410d -lopencv_features2d2410d -lopencv_nonfree2410d -lopencv_flann2410d
}
