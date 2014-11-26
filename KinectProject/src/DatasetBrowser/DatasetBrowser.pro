!include(../common.pri) {
    error("Couldn't find the common.pri file!")
}

QT       += core gui quick multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DatasetBrowser
TEMPLATE = app
CONFIG += link_prl

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
    #load(moc)
    INCLUDEPATH += $$PWD

    # CoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib

    # CoreLib Dynamic
    LIBS += -L$$BIN_PATH/ -lCoreLib

    # CoreLib Static
    PRE_TARGETDEPS += $$BIN_PATH/CoreLib.lib
}

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
