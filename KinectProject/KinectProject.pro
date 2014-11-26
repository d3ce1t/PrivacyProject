TEMPLATE = subdirs
CONFIG += ordered

DESTDIR = $$PWD/bin

SUBDIRS += src/CoreLib \
    src/PrivacyFilterLib \
    src/DatasetBrowser \
    src/DatasetParser \
    src/PrivacyFilters \
    src/PersonReid \
    src/PrivacyEditor
