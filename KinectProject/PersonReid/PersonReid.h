#ifndef PERSONREID_H
#define PERSONREID_H

#include <QObject>
#include "Feature.h"
#include "types/ColorFrame.h"
#include "ml/KMeans.h"

namespace dai {

class PersonReid : public QObject
{
    Q_OBJECT

public:
     shared_ptr<Feature> featureExtraction(shared_ptr<ColorFrame> colorFrame, const InstanceInfo& instance_info) const;

public slots:
    void execute();

private:
    void printClusters(const QList<Cluster<Feature>>& clusters) const;

};

} // End Namespace

#endif // PERSONREID_H
