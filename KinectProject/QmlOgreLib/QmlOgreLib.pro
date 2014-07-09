CONFIG += qt
QT += qml quick

TARGET = QmlOgre
TEMPLATE = lib
CONFIG += staticlib
CONFIG += create_prl

# Use C++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS = -std=c++11

UI_DIR = ./.ui
OBJECTS_DIR = ./.obj
MOC_DIR = ./.moc

SOURCES += ogreitem.cpp \
    ogrecamerawrapper.cpp \
    ogreengine.cpp \
    ogrerenderer.cpp

HEADERS += \
    ogreitem.h \
    ogrecamerawrapper.h \
    ogreengine.h \
    ogrerenderer.h

RESOURCES += \
    resources/ogrelib.qrc


unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += OGRE

    # Boost
    LIBS += -lboost_system

} else:win32 {

    # ensure QMAKE_MOC contains the moc executable path
    load(moc)

    OGREDIR = $$(OGRE_HOME)
    isEmpty(OGREDIR) {
        error(QmlOgreLib needs Ogre to be built. Please set the environment variable OGRE_HOME pointing to your Ogre root directory.)
    } else {
        INCLUDEPATH += $$OGREDIR/include/OGRE
        INCLUDEPATH += $$OGREDIR/include/OGRE/RenderSystems/GL
        CONFIG(release, debug|release) {
            LIBS += -L$$OGREDIR/lib/release -L$$OGREDIR/lib/release/opt -lOgreMain -lRenderSystem_GL
        } else {
            LIBS += -L$$OGREDIR/lib/debug -L$$OGREDIR/lib/debug/opt -lOgreMain_d -lRenderSystem_GL_d
        }
    }

    # Boost
    BOOSTDIR = $$(BOOST_ROOT)
    BOOSTLIB = $$(BOOST_LIBRARY)
    !isEmpty(BOOSTDIR) {
        INCLUDEPATH += $$BOOSTDIR
        CONFIG(release, debug|release) {
            LIBS += -L$$BOOSTLIB -llibboost_date_time-vc110-mt-1_55 -llibboost_thread-vc110-mt-1_55
        } else {
            LIBS += -L$$BOOSTLIB -llibboost_date_time-vc110-mt-gd-1_55 -llibboost_thread-vc110-mt-gd-1_55
        }
     }
}

# Copy all headers to build folder
#Headers.path = $$OUT_PWD/include
#Headers.files = $$files(*.h)
#INSTALLS += Headers
