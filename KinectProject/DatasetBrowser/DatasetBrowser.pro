#-------------------------------------------------
#
# Project created by QtCreator 2013-09-18T13:29:56
#
#-------------------------------------------------

QT       += core gui quick multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DatasetBrowser
TEMPLATE = app
CONFIG += link_prl

# Use C++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS = -std=c++11

HEADERS  += \
    DatasetBrowser.h \
    SettingsDialog.h \
    DatasetSelector.h

SOURCES += main.cpp\
    DatasetBrowser.cpp \
    SettingsDialog.cpp \
    DatasetSelector.cpp

OTHER_FILES += \
    qml/main.qml \
    glsl/scene3d.fsh \
    glsl/scene3d.vsh \
    glsl/scene2d.fsh \
    glsl/scene2d.vsh \
    glsl/skeleton.fsh \
    glsl/skeleton.vsh \
    glsl/silhouette.fsh \
    glsl/silhouette.vsh \
    data/MSRDailyActivity3D.xml \
    data/MSRAction3D.xml

RESOURCES += \
    openglunderqml.qrc

FORMS    += \
    DatasetBrowser.ui \
    SettingsDialog.ui \
    DatasetSelector.ui

# Linux
unix:!macx {
    # CoreLib
    LIBS += -L$$OUT_PWD/../CoreLib/ -lCoreLib
    PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/libCoreLib.a
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib
}

win32 {
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
}
