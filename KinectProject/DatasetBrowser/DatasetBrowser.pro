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
CONFIG += c++11

HEADERS  += \
    DatasetBrowser.h \
    SettingsDialog.h \
    DatasetSelector.h

SOURCES += main.cpp\
    DatasetBrowser.cpp \
    SettingsDialog.cpp \
    DatasetSelector.cpp

OTHER_FILES += \
    glsl/scene3d.fsh \
    glsl/scene3d.vsh \
    glsl/scene2d.fsh \
    glsl/scene2d.vsh \
    glsl/silhouette.fsh \
    glsl/silhouette.vsh

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
    CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/release/CoreLib.lib
    else:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/debug/CoreLib.lib
}

CONFIG(release, debug|release): DESTDIR = $$OUT_PWD/release
else:CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD/debug

# Install Linux Files
unix:!macx {
    # Config Files
    Config.path = $$OUT_PWD
    Config.files = ../config/linux/*
}

# Install Win Files
win32 {
    # Config Files
    Config.path = $$DESTDIR
    CONFIG(release, debug|release):Config.files = ../config/win/release/*
    else:CONFIG(debug, debug|release):Config.files = ../config/win/debug/*

    # OpenNI dll
    win32:OPENNI_DIR = $$(OPENNI2_REDIST)
    win32:OpenNI.path = $$DESTDIR
    win32:OpenNI.files = $$OPENNI_DIR/OpenNI2.dll $$OPENNI_DIR/OpenNI2

    # NiTE dll
    NITE_DIR = $$(NITE2_REDIST)
    NiTE.path = $$DESTDIR
    NiTE.files = $$NITE_DIR/NiTE2.dll

    INSTALLS += OpenNI NiTE
}

# make install
INSTALLS += Config
