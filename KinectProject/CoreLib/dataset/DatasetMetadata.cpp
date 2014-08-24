#include "DatasetMetadata.h"
#include "dataset/Dataset.h"
#include <QFile>
#include <QXmlStreamReader>
#include <iostream>
#include <QDebug>

using namespace std;

namespace dai {

const QList<QList<QString>> DatasetMetadata::ANY_LABEL = QList<QList<QString>>();
const QList<int> DatasetMetadata::ALL_ACTORS = QList<int>();

DatasetMetadata::~DatasetMetadata()
{
    m_instances.clear();
}

const QString DatasetMetadata::getName() const
{
    return m_name;
}

const QString DatasetMetadata::getDescription() const
{
    return m_description;
}

const QString DatasetMetadata::getPath() const
{
    return m_path;
}

const shared_ptr<InstanceInfo> DatasetMetadata::instance(int actor, int camera, int sample, const QList<QString> &labels)
{
    shared_ptr<InstanceInfo> result = nullptr;
    bool found = false;

    auto it = m_instances.constBegin();

    while (!found && it != m_instances.constEnd())
    {
        shared_ptr<InstanceInfo> instance = *it;

        if (instance->getActor() == actor && instance->getCamera() == camera &&
                instance->getSample() == sample && instance->hasLabels(labels)) {
            result = instance;
            found = true;
        }

        ++it;
    }

    return result;
}

const shared_ptr<InstanceInfo> DatasetMetadata::instance(int actor, int camera, int sample, const QList<QString>& labels, DataFrame::FrameType type)
{
    shared_ptr<InstanceInfo> result = nullptr;
    bool found = false;

    QListIterator<shared_ptr<InstanceInfo>> it(m_instances);

    while (!found && it.hasNext())
    {
        shared_ptr<InstanceInfo> instance = it.next();

        if (instance->getActor() == actor && instance->getCamera() == camera &&
                instance->getSample() == sample && instance->hasLabels(labels) &&
                instance->getType().testFlag(type)) {
            result = instance;
            found = true;
        }
    }

    if (!found) {
        throw 1;
    }

    return result;
}

/*const QList<shared_ptr<InstanceInfo>> DatasetMetadata::instance(int actor, int camera, int sample)
{
    QList<shared_ptr<InstanceInfo>> result;

    auto it = m_instances.constBegin();

    while (it != m_instances.constEnd())
    {
        shared_ptr<InstanceInfo> instance = *it;

        if (instance->getActor() == actor && instance->getCamera() == camera &&
                instance->getSample() == sample) {
            result << instance;
        }

        ++it;
    }

    return result;
}*/

const QList<shared_ptr<InstanceInfo>> DatasetMetadata::instances(DataFrame::FrameType type,
                                                      const QList<int>& actors,
                                                      const QList<int>& cameras,
                                                      const QList<QList<QString>>& labels) const
{
    QList<shared_ptr<InstanceInfo>> result;

    if (m_instances.isEmpty())
        return result;

    auto it1 = m_instances.constBegin();

    while (it1 != m_instances.constEnd())
    {
        shared_ptr<InstanceInfo> instance = *it1;
        bool isActorChecked = false;
        bool isCameraChecked = false;
        bool isLabelsChecked = false;

        if (actors.isEmpty() || actors.contains(instance->getActor()))
            isActorChecked = true;

        if (cameras.isEmpty() || cameras.contains(instance->getCamera()))
            isCameraChecked = true;

        if (labels.isEmpty())
            isLabelsChecked = true;
        else
        {
            bool found = false;
            auto it2 = labels.constBegin();

            while (it2 != labels.constEnd() && !found)
            {
                if (instance->hasLabels(*it2)) {
                    isLabelsChecked = true;
                    found = true;
                }

                ++it2;
            }
        }

        if (instance->getType().testFlag(type) && isActorChecked && isCameraChecked && isLabelsChecked) {
            result.append(instance);
        }

        ++it1;
    }

    return result;
}

const QMap<int, QString>& DatasetMetadata::actors() const
{
    return m_actors;
}

const QMap<int, QString>& DatasetMetadata::cameras() const
{
    return m_cameras;
}

const QMap<QString, QString>& DatasetMetadata::labels() const
{
    return m_labels;
}

void DatasetMetadata::addInstanceInfo(shared_ptr<InstanceInfo> instance)
{
    Q_ASSERT(instance != nullptr);
    m_instances.push_back(instance);
}

void DatasetMetadata::setDataset(Dataset* dataset)
{
    m_dataset = dataset;
}

const Dataset &DatasetMetadata::dataset() const
{
    return *m_dataset;
}

const DataFrame::SupportedFrames DatasetMetadata::availableInstanceTypes() const
{
    return m_availableInstanceTypes;
}

shared_ptr<DatasetMetadata> DatasetMetadata::load(QString xmlPath)
{
    shared_ptr<DatasetMetadata> dsMetaDataObject(new DatasetMetadata());
    QFile file(xmlPath);
    file.open(QIODevice::ReadOnly);
    int version = 0;

    QXmlStreamReader reader;
    reader.setDevice(&file);

    // Read Version
    while (!reader.atEnd() && version == 0) {
        QXmlStreamReader::TokenType type = reader.readNext();
        if (type == QXmlStreamReader::StartElement && reader.name() == "dataset") {
            version = reader.attributes().value("need-version").toInt();
        }
    }

    if (reader.hasError()) {
        cerr << "Error parsing dataset: " << endl;
        cerr << reader.errorString().toStdString() << endl;
    }

    reader.clear();
    file.close();

    if (version == 1)
        dsMetaDataObject = load_version1(xmlPath);
    else if (version == 2)
        dsMetaDataObject = load_version2(xmlPath);
    else
        dsMetaDataObject = load_version3(xmlPath);

    return dsMetaDataObject;
}

shared_ptr<DatasetMetadata> DatasetMetadata::load_version1(QString xmlPath)
{
    shared_ptr<DatasetMetadata> dsMetaDataObject(new DatasetMetadata());
    QFile file(xmlPath);
    file.open(QIODevice::ReadOnly);

    QXmlStreamReader reader;
    reader.setDevice(&file);

    bool insideActivities = false;
    bool insideSamples = false;
    bool insideActors = false;
    bool insideInstances = false;

    while (!reader.atEnd())
    {
        QXmlStreamReader::TokenType type = reader.readNext();

        switch (type) {
        case QXmlStreamReader::StartElement:
            // DataSet tag
            if (reader.name() == "dataset") {
                dsMetaDataObject->m_name = reader.attributes().value("name").toString();
            }
            // Description tag
            else if (reader.name() == "description") {
                reader.readNext();
                dsMetaDataObject->m_description = reader.text().toString().trimmed();
            }
            // Path tag
            else if (reader.name() == "path") {
                reader.readNext();
                dsMetaDataObject->m_path = reader.text().toString().trimmed();
            }
            // Activities tag
            else if (reader.name() == "activities") {
                //int number = reader.attributes().value("size").toString().toInt();
                //dsMetaDataObject->m_numberOfLabels = number;
                insideActivities = true;
            }
            // Activity tag
            else if (reader.name() == "activity" && insideActivities) {
                QString key = "act" + reader.attributes().value("key").toString();
                reader.readNext();
                dsMetaDataObject->m_labels[key] = reader.text().toString();
            }
            // Actors tag
            else if (reader.name() == "actors") {
                //int number = reader.attributes().value("size").toString().toInt();
                //dsMetaDataObject->m_numberOfActors = number;
                insideActors = true;
            }
            // Actor tag
            else if (reader.name() == "actor" && insideActors) {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                dsMetaDataObject->m_actors[key] = reader.text().toString();
            }
            // Samples tag
            else if (reader.name() == "samples") {
                //int number = reader.attributes().value("size").toString().toInt();
                //dsMetaDataObject->m_numberOfSampleTypes = number;
                insideSamples = true;
            }
            // Sample tag
            else if (reader.name() == "sample" && insideSamples) {
                QString key = "rep" + reader.attributes().value("key").toString();
                reader.readNext();
                dsMetaDataObject->m_labels[key] = reader.text().toString();
            }
            // Instances tag
            else if (reader.name() == "instances") {
                insideInstances = true;
            }
            // Instance tag
            else if (reader.name() == "instance" && insideInstances) {
                QString strType = reader.attributes().value("type").toString();
                QString activity = "act" + reader.attributes().value("activity").toString();
                int actor = reader.attributes().value("actor").toString().toInt();
                int sample = reader.attributes().value("sample").toString().toInt();
                QString strSample = "rep" + reader.attributes().value("sample").toString();

                reader.readNextStartElement(); // seek into <file>
                reader.readNext(); // seek into text
                QString file = reader.text().toString();

                DataFrame::FrameType type = DataFrame::Unknown;

                if (strType == "depth")
                    type = DataFrame::Depth;
                else if (strType == "color")
                    type = DataFrame::Color;
                else if (strType == "skeleton")
                    type = DataFrame::Skeleton;
                else if (strType == "user")
                    type = DataFrame::Mask;

                dsMetaDataObject->m_availableInstanceTypes |= type;

                shared_ptr<InstanceInfo> instanceInfo = dsMetaDataObject->instance(actor, 1, sample, {activity, strSample});

                if (!instanceInfo) {
                    instanceInfo = make_shared<InstanceInfo>(dsMetaDataObject);
                    instanceInfo->setActor(actor);
                    instanceInfo->setSample(sample);
                    instanceInfo->setCamera(1);
                    instanceInfo->addLabel(activity);
                    instanceInfo->addLabel(strSample);
                    // Insert
                    dsMetaDataObject->addInstanceInfo(instanceInfo);
                }

                instanceInfo->addType(type);
                instanceInfo->addFileName(type, file);
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name() == "activities") {
                insideActivities = false;
            }
            else if (reader.name() == "actors") {
                insideActors = false;
            }
            else if (reader.name() == "samples") {
                insideSamples = false;
            }
            else if (reader.name() == "instances") {
                insideInstances = false;
            }
            break;

        default:
            break;
        }
    }

    if (reader.hasError()) {
        cerr << "Error parsing MSRDailyActivity3D dataset: " << endl;
        cerr << reader.errorString().toStdString() << endl;
    }

    reader.clear();
    file.close();

    return dsMetaDataObject;
}

shared_ptr<DatasetMetadata> DatasetMetadata::load_version2(QString xmlPath)
{
    shared_ptr<DatasetMetadata> dsMetaDataObject(new DatasetMetadata());
    QFile file(xmlPath);
    file.open(QIODevice::ReadOnly);

    QXmlStreamReader reader;
    reader.setDevice(&file);

    bool insideActivities = false;
    bool insideSamples = false;
    bool insideActors = false;
    bool insideInstances = false;
    shared_ptr<InstanceInfo> instanceInfo = nullptr;
    QHash<int, QString> fileNames;

    while (!reader.atEnd())
    {
        QXmlStreamReader::TokenType type = reader.readNext();

        switch (type) {
        case QXmlStreamReader::StartElement:
            // DataSet tag
            if (reader.name() == "dataset") {
                dsMetaDataObject->m_name = reader.attributes().value("name").toString();
            }
            // Description tag
            else if (reader.name() == "description") {
                reader.readNext();
                dsMetaDataObject->m_description = reader.text().toString().trimmed();
            }
            // Path tag
            else if (reader.name() == "path") {
                reader.readNext();
                dsMetaDataObject->m_path = reader.text().toString().trimmed();
            }
            // Activities tag
            else if (reader.name() == "activities") {
                //int number = reader.attributes().value("size").toString().toInt();
                //dsMetaDataObject->m_numberOfLabels = number;
                insideActivities = true;
            }
            // Activity tag
            else if (reader.name() == "activity" && insideActivities) {
                QString key = "act" + reader.attributes().value("key").toString();
                reader.readNext();
                dsMetaDataObject->m_labels[key] = reader.text().toString();
            }
            // Actors tag
            else if (reader.name() == "actors") {
                //int number = reader.attributes().value("size").toString().toInt();
                //dsMetaDataObject->m_numberOfActors = number;
                insideActors = true;
            }
            // Actor tag
            else if (reader.name() == "actor" && insideActors) {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                dsMetaDataObject->m_actors[key] = reader.text().toString();
            }
            // Samples tag
            else if (reader.name() == "samples") {
                //int number = reader.attributes().value("size").toString().toInt();
                //dsMetaDataObject->m_numberOfSampleTypes = number;
                insideSamples = true;
            }
            // Sample tag
            else if (reader.name() == "sample" && insideSamples) {
                QString key = "rep" + reader.attributes().value("key").toString();
                reader.readNext();
                dsMetaDataObject->m_labels[key] = reader.text().toString();
            }
            // Instances tag
            else if (reader.name() == "instances") {
                insideInstances = true;
            }
            // Instance tag
            else if (reader.name() == "instance" && insideInstances) {
                /*
                 * <instance activity="1" actor="10" sample="1">
                 *      <file id="1">S5_C4_U10_L1.oni</file>
                 *      <data type="color" file-ref="1" />
                 *      <data type="depth" file-ref="1" />
                 *      <data type="skeleton" file-ref="1" />
                 *      <data type="mask" file-ref="1" />
                 * </instance>
                 */
                QString activity = "act" + reader.attributes().value("activity").toString();
                int actor = reader.attributes().value("actor").toString().toInt();
                int sample = reader.attributes().value("sample").toString().toInt();
                QString strSample = "rep" + reader.attributes().value("sample").toString();

                instanceInfo = dsMetaDataObject->instance(actor, 1, sample, {activity, strSample});

                if (instanceInfo) {
                    qDebug() << "Such an item should not exist yet";
                    throw 1;
                }

                instanceInfo = make_shared<InstanceInfo>(dsMetaDataObject);
                instanceInfo->setActor(actor);
                instanceInfo->setSample(sample);
                instanceInfo->setCamera(1);
                instanceInfo->addLabel(activity);
                instanceInfo->addLabel(strSample);
                // Insert
                dsMetaDataObject->addInstanceInfo(instanceInfo);
            }
            else if (reader.name() == "file" && insideInstances && instanceInfo != nullptr) {
                int fileId = reader.attributes().value("id").toString().toInt();
                reader.readNext();
                QString file = reader.text().toString();
                fileNames.insert(fileId, file);
            }
            else if (reader.name() == "data" && insideInstances && instanceInfo != nullptr) {
                QString strType = reader.attributes().value("type").toString();
                int fileRef = reader.attributes().value("file-ref").toInt();
                DataFrame::FrameType type = DataFrame::Unknown;

                if (strType == "depth")
                    type = DataFrame::Depth;
                else if (strType == "color")
                    type = DataFrame::Color;
                else if (strType == "skeleton")
                    type = DataFrame::Skeleton;
                else if (strType == "mask")
                    type = DataFrame::Mask;

                dsMetaDataObject->m_availableInstanceTypes |= type;
                instanceInfo->addType(type);
                instanceInfo->addFileName(type, fileNames.value(fileRef));
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name() == "activities") {
                insideActivities = false;
            }
            else if (reader.name() == "actors") {
                insideActors = false;
            }
            else if (reader.name() == "samples") {
                insideSamples = false;
            }
            else if (reader.name() == "instances") {
                insideInstances = false;
            }
            else if (reader.name() == "instance" && insideInstances) {
                instanceInfo = nullptr;
                fileNames.clear();
            }
            break;

        default:
            break;
        }
    }

    if (reader.hasError()) {
        cerr << "Error parsing dataset: " << endl;
        cerr << reader.errorString().toStdString() << endl;
    }

    reader.clear();
    file.close();

    return dsMetaDataObject;
}

shared_ptr<DatasetMetadata> DatasetMetadata::load_version3(QString xmlPath)
{
    shared_ptr<DatasetMetadata> dsMetaDataObject(new DatasetMetadata());
    QFile file(xmlPath);
    file.open(QIODevice::ReadOnly);

    QXmlStreamReader reader;
    reader.setDevice(&file);

    bool insideCameras = false;
    bool insideActors = false;
    bool insideLabels = false;
    bool insideInstances = false;
    shared_ptr<InstanceInfo> instanceInfo = nullptr;
    QHash<int, QString> fileNames;

    while (!reader.atEnd())
    {
        QXmlStreamReader::TokenType type = reader.readNext();

        switch (type) {
        case QXmlStreamReader::StartElement:
            // DataSet tag
            if (reader.name() == "dataset") {
                dsMetaDataObject->m_name = reader.attributes().value("name").toString();
            }
            // Description tag
            else if (reader.name() == "description") {
                reader.readNext();
                dsMetaDataObject->m_description = reader.text().toString().trimmed();
            }
            // Path tag
            else if (reader.name() == "path") {
                reader.readNext();
                dsMetaDataObject->m_path = reader.text().toString().trimmed();
            }
            // Cameras tag
            else if (reader.name() == "cameras") {
                insideCameras = true;
            }
            // Camera tag
            else if (reader.name() == "camera") {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                dsMetaDataObject->m_cameras[key] = reader.text().toString();
            }
            // Actors tag
            else if (reader.name() == "actors") {
                //int number = reader.attributes().value("size").toString().toInt();
                //dsMetaDataObject->m_numberOfActors = number;
                insideActors = true;
            }
            // Actor tag
            else if (reader.name() == "actor" && insideActors) {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                dsMetaDataObject->m_actors[key] = reader.text().toString();
            }
            // Labels tag
            else if (reader.name() == "labels") {
                //int number = reader.attributes().value("size").toString().toInt();
                //dsMetaDataObject->m_numberOfLabels = number;
                insideLabels = true;
            }
            // Label tag
            else if (reader.name() == "label" && insideLabels) {
                QString key = reader.attributes().value("key").toString();
                reader.readNext();
                dsMetaDataObject->m_labels[key] = reader.text().toString();
            }
            // Instances tag
            else if (reader.name() == "instances") {
                insideInstances = true;
            }
            // Instance tag
            else if (reader.name() == "instance" && insideInstances) {
                /*
                 * <instance activity="1" actor="10" sample="1">
                 *      <file id="1">S5_C4_U10_L1.oni</file>
                 *      <data type="color" file-ref="1" />
                 *      <data type="depth" file-ref="1" />
                 *      <data type="skeleton" file-ref="1" />
                 *      <data type="mask" file-ref="1" />
                 * </instance>
                 */
                int actor = reader.attributes().value("actor").toString().toInt();
                int camera = reader.attributes().value("camera").toString().toInt();
                int sample = reader.attributes().value("sample").toString().toInt();
                QStringList labels = reader.attributes().value("label").toString().split(',');

                instanceInfo = dsMetaDataObject->instance(actor, camera, sample, labels);

                if (instanceInfo) {
                    qDebug() << "Such an item should not exist yet";
                    throw 1;
                }

                instanceInfo = make_shared<InstanceInfo>(dsMetaDataObject);
                instanceInfo->setActor(actor);
                instanceInfo->setCamera(camera);
                instanceInfo->setSample(sample);
                foreach (auto label, labels)
                    instanceInfo->addLabel(label);
                // Insert
                dsMetaDataObject->addInstanceInfo(instanceInfo);
            }
            else if (reader.name() == "file" && insideInstances && instanceInfo != nullptr) {
                int fileId = reader.attributes().value("id").toString().toInt();
                reader.readNext();
                QString file = reader.text().toString();
                fileNames.insert(fileId, file);
            }
            else if (reader.name() == "data" && insideInstances && instanceInfo != nullptr) {
                QString strType = reader.attributes().value("type").toString();
                int fileRef = reader.attributes().value("file-ref").toInt();
                DataFrame::FrameType type = DataFrame::Unknown;

                if (strType == "depth")
                    type = DataFrame::Depth;
                else if (strType == "color")
                    type = DataFrame::Color;
                else if (strType == "skeleton")
                    type = DataFrame::Skeleton;
                else if (strType == "mask")
                    type = DataFrame::Mask;

                dsMetaDataObject->m_availableInstanceTypes |= type;
                instanceInfo->addType(type);
                instanceInfo->addFileName(type, fileNames.value(fileRef));
            }
            break;

        case QXmlStreamReader::EndElement:
            if (reader.name() == "cameras") {
                insideCameras = false;
            }
            else if (reader.name() == "actors") {
                insideActors = false;
            }
            else if (reader.name() == "labels") {
                insideLabels = false;
            }
            else if (reader.name() == "instances") {
                insideInstances = false;
            }
            else if (reader.name() == "instance" && insideInstances) {
                instanceInfo = nullptr;
                fileNames.clear();
            }
            break;

        default:
            break;
        }

        if (reader.hasError()) {
            cerr << "Error parsing dataset (" << reader.lineNumber() << "," << reader.columnNumber() << ")" << endl;
            cerr << reader.errorString().toStdString() << endl;
        }
    }

    reader.clear();
    file.close();

    return dsMetaDataObject;
}

} // End Namespace
