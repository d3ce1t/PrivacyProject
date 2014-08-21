#-------------------------------------------------
#
# Project created by QtCreator 2014-08-19T15:38:43
#
#-------------------------------------------------

QT       += core quick gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PersonReid
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += link_prl
CONFIG += c++11

TEMPLATE = app

# Fix for Qt 5.3.1 on VS 2013 compiler, because Qt does not define a macro for this compiler
DEFINES += Q_COMPILER_INITIALIZER_LISTS


HEADERS += \
    PersonReid.h \
    Feature.h

SOURCES += main.cpp \
    PersonReid.cpp \
    Feature.cpp


unix:!macx {
    # CoreLib
    LIBS += -L$$OUT_PWD/../CoreLib/ -lCoreLib
    PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/libCoreLib.a
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib
}

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
    win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/release/libCoreLib.a
    else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/debug/libCoreLib.a
    else:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/release/CoreLib.lib
    else:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/debug/CoreLib.lib

    # Boost
    BOOSTDIR = $$(BOOST_INCLUDEDIR)
    BOOSTLIB = $$(BOOST_LIBRARYDIR)
    !isEmpty(BOOSTDIR) {
        INCLUDEPATH += $$BOOSTDIR
        win32-g++:CONFIG(release, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-mgw48-mt-1_55 -lboost_thread-mgw48-mt-1_55
        else:win32-g++:CONFIG(debug, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-mgw48-mt-d-1_55 -lboost_thread-mgw48-mt-d-1_55
        else:CONFIG(release, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-vc120-mt-1_55 -lboost_thread-vc120-mt-1_55
        else:CONFIG(debug, debug|release):LIBS += -L$$BOOSTLIB -lboost_date_time-vc120-mt-gd-1_55 -lboost_thread-vc120-mt-gd-1_55
    }

    # OpenCV2
    INCLUDEPATH += $$(OPENCV2_INCLUDE)
    DEPENDPATH += $$(OPENCV2_INCLUDE)
    CONFIG(release, debug|release):LIBS += -L$$(OPENCV2_LIB) -lopencv_core249 -lopencv_imgproc249 -lopencv_highgui249 -lopencv_objdetect249 -lopencv_photo249
    else:CONFIG(debug, debug|release):LIBS += -L$$(OPENCV2_LIB) -lopencv_core249d -lopencv_imgproc249d -lopencv_highgui249d -lopencv_objdetect249d -lopencv_photo249d
}
