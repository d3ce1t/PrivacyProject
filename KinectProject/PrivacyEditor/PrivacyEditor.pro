#-------------------------------------------------
#
# Project created by QtCreator 2014-11-18T11:07:29
#
#-------------------------------------------------

QT       += core gui quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PrivacyEditor
TEMPLATE = app
CONFIG += c++11


SOURCES += main.cpp\
        MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

RESOURCES += \
    resources.qrc

win32 {
    # ensure QMAKE_MOC contains the moc executable path
    load(moc)

    INCLUDEPATH += $$PWD

    # CoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # CoreLib Dynamic
    CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../CoreLib/release/ -lCoreLib
    else:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../CoreLib/debug/ -lCoreLib

    # CoreLib Static
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/release/CoreLib.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/debug/CoreLib.lib

    # PrivacyFiltersLib
    INCLUDEPATH += $$PWD/../PrivacyFilterLib
    DEPENDPATH += $$PWD/../PrivacyFilterLib

    # PrivacyFiltersLib Dynamic
    CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../PrivacyFilterLib/release/ -lPrivacyFilterLib
    else:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../PrivacyFilterLib/debug/ -lPrivacyFilterLib

    # PrivacyFiltersLib Static
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../PrivacyFilterLib/release/PrivacyFilterLib.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../PrivacyFilterLib/debug/PrivacyFilterLib.lib

    # Ogre
    #OGREDIR = $$(OGRE_HOME)
    #isEmpty(OGREDIR) {
    #    error(PrivacyFilters needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    #} else {
    #    INCLUDEPATH += $$OGREDIR/include/OGRE
    #    INCLUDEPATH += $$OGREDIR/include/OGRE/RenderSystems/GL
    #    CONFIG(release, debug|release):LIBS += -L$$OGREDIR/lib/Release -L$$OGREDIR/lib/Release/opt -lOgreMain -lRenderSystem_GL
    #    else:CONFIG(debug, debug|release):LIBS += -L$$OGREDIR/lib/Rebug -L$$OGREDIR/lib/Debug/opt -lOgreMain_d -lRenderSystem_GL_d
    #}

    # Boost
    #BOOSTDIR = $$(BOOST_INCLUDEDIR)
    #BOOSTLIB = $$(BOOST_LIBRARYDIR)
    #!isEmpty(BOOSTDIR) {
    #    INCLUDEPATH += $$BOOSTDIR
    #    win32-g++:CONFIG(release, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-mgw48-mt-1_56 -lboost_thread-mgw48-mt-1_56
    #    else:win32-g++:CONFIG(debug, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-mgw48-mt-d-1_56 -lboost_thread-mgw48-mt-d-1_56
    #    else:CONFIG(release, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-vc120-mt-1_56 -lboost_thread-vc120-mt-1_56
    #    else:CONFIG(debug, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-vc120-mt-gd-1_56 -lboost_thread-vc120-mt-gd-1_56
    #}

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
