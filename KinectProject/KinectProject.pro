TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += src/CoreLib \
    src/PrivacyFilterLib \
    src/DatasetBrowser \
    src/DatasetParser \
    src/PrivacyFilters \
    src/PersonReid \
    src/PrivacyEditor

# Install script
DESTDIR = $$OUT_PWD/bin

# Ogre Resources
Resources.path = $$DESTDIR/resources
Resources.files = $$PWD/OgreData/*

# Install Linux Files
unix:!macx {
    # Config Files
    Config.path = $$DESTDIR
    Config.files = $$PWD/config/linux/*
}

# Install Win Files
win32 {
    # Config Files
    Config.path = $$DESTDIR
    CONFIG(release, debug|release):Config.files = $$PWD/config/win/release/*
    else:CONFIG(debug, debug|release):Config.files = $$PWD/config/win/debug/*

    # OpenCV dll
    OpenCV.path = $$DESTDIR
    CONFIG(release, debug|release):OpenCV.files = $$(OPENCV2_BIN)/opencv_core2410.dll $$(OPENCV2_BIN)/opencv_imgproc2410.dll $$(OPENCV2_BIN)/opencv_highgui2410.dll $$(OPENCV2_BIN)/opencv_objdetect2410.dll $$(OPENCV2_BIN)/opencv_photo2410.dll
    else:CONFIG(debug, debug|release):OpenCV.files = $$(OPENCV2_BIN)/opencv_core2410d.dll $$(OPENCV2_BIN)/opencv_imgproc2410d.dll $$(OPENCV2_BIN)/opencv_highgui2410d.dll $$(OPENCV2_BIN)/opencv_objdetect2410d.dll $$(OPENCV2_BIN)/opencv_photo2410d.dll

    # Ogre dll
    Ogre.path = $$DESTDIR
    CONFIG(release, debug|release) {
        OGRE_DIR = $$(OGRE_HOME)\bin\release
        Ogre.files = $$OGRE_DIR/OgreMain.dll $$OGRE_DIR/RenderSystem_GL.dll
    }
    else:CONFIG(debug, debug|release) {
        OGRE_DIR = $$(OGRE_HOME)\bin\debug
        Ogre.files = $$OGRE_DIR/OgreMain_d.dll $$OGRE_DIR/RenderSystem_GL_d.dll
    }

    # OpenNI dll
    win32:OPENNI_DIR = $$(OPENNI2_REDIST)
    win32:OpenNI.path = $$DESTDIR
    win32:OpenNI.files = $$OPENNI_DIR/OpenNI2.dll $$OPENNI_DIR/OpenNI2

    # NiTE dll
    NITE_DIR = $$(NITE2_REDIST)
    NiTE.path = $$DESTDIR
    NiTE.files = $$NITE_DIR/NiTE2.dll

    INSTALLS += OpenCV Ogre OpenNI NiTE
}

# make install
INSTALLS += Resources Config
