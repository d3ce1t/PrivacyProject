#ifndef QMLENUMSWRAPPER_H
#define QMLENUMSWRAPPER_H

#include <QObject>

class QMLEnumsWrapper : public QObject
{
    Q_OBJECT
    Q_ENUMS(ColorFilter)

public:
    enum ColorFilter {
        FILTER_DISABLED = 0,
        FILTER_INVISIBILITY,
        FILTER_BLUR,
        FILTER_PIXELATION,
        FILTER_EMBOSS,
        FILTER_SILHOUETTE,
        FILTER_SKELETON,
        FILTER_3DMODEL
    };
};

#endif // QMLENUMSWRAPPER_H
