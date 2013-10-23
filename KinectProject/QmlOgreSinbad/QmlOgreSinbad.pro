CONFIG += qt
QT += qml quick
TEMPLATE = app
TARGET = qmlogre

UI_DIR = ./.ui
OBJECTS_DIR = ./.obj
MOC_DIR = ./.moc

# Use C++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS = -std=c++11

SOURCES += main.cpp \
    cameranodeobject.cpp \
    exampleapp.cpp \
    openni/OpenNIRuntime.cpp \
    character/SinbadCharacterController.cpp

HEADERS += cameranodeobject.h \
    exampleapp.h \
    openni/OpenNIRuntime.h \
    character/SinbadCharacterController.h

OTHER_FILES += \
    resources/example.qml

macx {
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error(QmlOgre needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    } else {
        message(Using Ogre libraries in $$OGREDIR)
        INCLUDEPATH += $$OGREDIR/include/OGRE
        INCLUDEPATH += $$OGREDIR/include/OGRE/RenderSystems/GL
        QMAKE_LFLAGS += -F$$OGREDIR/lib/release
        LIBS += -framework Ogre

        BOOSTDIR = $$OGREDIR/boost_1_42
        !isEmpty(BOOSTDIR) {
            INCLUDEPATH += $$BOOSTDIR
#            LIBS += -L$$BOOSTDIR/lib -lboost_date_time-xgcc40-mt-1_42 -lboost_thread-xgcc40-mt-1_42
        }
    }
} else:unix {
    # OpenNI2
    LIBS += -L/opt/OpenNI-Linux-x64-2.2/Tools/ -lOpenNI2
    INCLUDEPATH += /opt/OpenNI-Linux-x64-2.2/Include
    DEPENDPATH += /opt/OpenNI-Linux-x64-2.2/Include

    # NiTE2
    LIBS += -L/opt/NiTE-Linux-x64-2.2/Redist/ -lNiTE2
    INCLUDEPATH += /opt/NiTE-Linux-x64-2.2/Include
    DEPENDPATH += /opt/NiTE-Linux-x64-2.2/Include

    # QmlOgreLib
    LIBS += -L$$OUT_PWD/../QmlOgreLib/ -lQmlOgre
    PRE_TARGETDEPS += $$OUT_PWD/../QmlOgreLib/libQmlOgre.a
    INCLUDEPATH += $$PWD/../QmlOgreLib
    DEPENDPATH += $$PWD/../QmlOgreLib

    # Ogre
    CONFIG += link_pkgconfig
    PKGCONFIG += OGRE

    # Boost
    LIBS += -lboost_system

    # CoreLib
    unix:!macx: LIBS += -L$$OUT_PWD/../CoreLib/ -lCoreLib
    INCLUDEPATH += $$PWD/../CoreLib
    DEPENDPATH += $$PWD/../CoreLib
    unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../CoreLib/libCoreLib.a
} else:win32 {
    # Ogre
    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error(QmlOgre needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    } else {
        message(Using Ogre libraries in $$OGREDIR)
        INCLUDEPATH += $$OGREDIR/include/OGRE
        INCLUDEPATH += $$OGREDIR/include/OGRE/RenderSystems/GL
        CONFIG(release, debug|release) {
            LIBS += -L$$OGREDIR/lib/release -L$$OGREDIR/lib/release/opt -lOgreMain -lRenderSystem_GL
        } else {
            LIBS += -L$$OGREDIR/lib/debug -L$$OGREDIR/lib/debug/opt -lOgreMain_d -lRenderSystem_GL_d
        }

        BOOSTDIR = $$OGREDIR/boost_1_42
        !isEmpty(BOOSTDIR) {
            INCLUDEPATH += $$BOOSTDIR
            CONFIG(release, debug|release) {
                LIBS += -L$$BOOSTDIR/lib -llibboost_date_time-vc90-mt-1_42 -llibboost_thread-vc90-mt-1_42
            } else {
                LIBS += -L$$BOOSTDIR/lib -llibboost_date_time-vc90-mt-gd-1_42 -llibboost_thread-vc90-mt-gd-1_42
            }
        }
    }

    # QmlOgreLib Dynamic
    win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QmlOgreLib/release/ -lQmlOgre
    else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QmlOgreLib/debug/ -lQmlOgre

    # QmlOgreLib Static
    win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QmlOgreLib/release/QmlOgre.lib
    else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../QmlOgreLib/debug/QmlOgre.lib
}

RESOURCES += resources/resources.qrc

# Copy all resources to build folder
Resources.path = $$OUT_PWD/resources
Resources.files = ../OgreData/*

# Copy all config files to build folder
Config.path = $$OUT_PWD
Config.files = config/*

# make install
INSTALLS += Resources Config
