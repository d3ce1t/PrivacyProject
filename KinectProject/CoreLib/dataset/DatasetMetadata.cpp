#include "DatasetMetadata.h"
#include "dataset/Dataset.h"
#include <QFile>
#include <QXmlStreamReader>
#include <iostream>
#include <QDebug>

using namespace dai;
using namespace std;

DatasetMetadata::DatasetMetadata()
{
}

DatasetMetadata::~DatasetMetadata()
{
    foreach (QString* str, m_activities) {
        delete str;
    }
    m_activities.clear();

    foreach (QString* str, m_actors) {
        delete str;
    }
    m_actors.clear();

    foreach (QString* str, m_sampleTypes) {
        delete str;
    }
    m_sampleTypes.clear();

    // Free Memory
    foreach (InstanceInfo* instance, m_instances) {
        delete instance;
    }

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

int DatasetMetadata::getNumberOfActivities() const
{
    return m_numberOfActivities;
}

int DatasetMetadata::getNumberOfActors() const
{
    return m_numberOfActors;
}

int DatasetMetadata::getNumberOfSampleTypes() const
{
    return m_numberOfSampleTypes;
}

const InstanceInfo* DatasetMetadata::instance(DataFrame::FrameType type, int activity, int actor, int sample)
{
    InstanceInfo* result = nullptr;
    bool found = false;

    QListIterator<InstanceInfo*> it(m_instances);

    while (!found && it.hasNext())
    {
        InstanceInfo* instance = it.next();

        if (instance->getType().testFlag(type) && instance->getActivity() == activity &&
            instance->getActor() == actor && instance->getSample() == sample) {
            found = true;
            result = instance;
        }
    }

    if (!found) {
        throw 1;
    }

    return result;
}

const InstanceInfo* DatasetMetadata::instance(int activity, int actor, int sample)
{
    InstanceInfo* result = nullptr;
    bool found = false;

    QListIterator<InstanceInfo*> it(m_instances);

    while (!found && it.hasNext())
    {
        InstanceInfo* instance = it.next();

        if (instance->getActivity() == activity && instance->getActor() == actor && instance->getSample() == sample) {
            found = true;
            result = instance;
        }
    }

    return result;
}

const InstanceInfoList DatasetMetadata::instances(
        DataFrame::FrameType type, const QList<int>* activities, const QList<int>* actors, const QList<int>* samples) const
{
    InstanceInfoList result;

    if (m_instances.isEmpty())
        return result;

    QListIterator<InstanceInfo*> it(m_instances);

    while (it.hasNext())
    {
        InstanceInfo* instance = it.next();
        bool isActivityChecked = false;
        bool isActorChecked = false;
        bool isSampleChecked = false;

        if (activities == 0 || (activities != 0 && activities->contains(instance->getActivity())))
            isActivityChecked = true;

        if (actors == 0 || (actors != 0 && actors->contains(instance->getActor())))
            isActorChecked = true;

        if (samples == 0 || (samples != 0 && samples->contains(instance->getSample())))
            isSampleChecked = true;

        if (instance->getType().testFlag(type) && isActivityChecked && isActorChecked && isSampleChecked) {
            result.append(instance);
        }
    }

    return result;
}

const QString& DatasetMetadata::getActivityName(int key) const
{
    return *(m_activities.value(key));
}

const QString& DatasetMetadata::getActorName(int key) const
{
    return *(m_actors.value(key));
}

const QString& DatasetMetadata::getSampleName(int key) const
{
    return *(m_sampleTypes.value(key));
}

const QList<int> DatasetMetadata::actors() const
{
    return m_actors.keys();
}

void DatasetMetadata::addInstanceInfo(InstanceInfo* instance)
{
    Q_ASSERT(instance != nullptr);
    m_instances.push_back(instance);
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
    else
        dsMetaDataObject = load_version2(xmlPath);

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
                int number = reader.attributes().value("size").toString().toInt();
                dsMetaDataObject->m_numberOfActivities = number;
                insideActivities = true;
            }
            // Activity tag
            else if (reader.name() == "activity" && insideActivities) {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                dsMetaDataObject->m_activities[key] = new QString(reader.text().toString());
            }
            // Actors tag
            else if (reader.name() == "actors") {
                int number = reader.attributes().value("size").toString().toInt();
                dsMetaDataObject->m_numberOfActors = number;
                insideActors = true;
            }
            // Actor tag
            else if (reader.name() == "actor" && insideActors) {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                dsMetaDataObject->m_actors[key] = new QString(reader.text().toString());
            }
            // Samples tag
            else if (reader.name() == "samples") {
                int number = reader.attributes().value("size").toString().toInt();
                dsMetaDataObject->m_numberOfSampleTypes = number;
                insideSamples = true;
            }
            // Sample tag
            else if (reader.name() == "sample" && insideSamples) {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                dsMetaDataObject->m_sampleTypes[key] = new QString(reader.text().toString());
            }
            // Instances tag
            else if (reader.name() == "instances") {
                insideInstances = true;
            }
            // Instance tag
            else if (reader.name() == "instance" && insideInstances) {
                QString strType = reader.attributes().value("type").toString();
                int activity = reader.attributes().value("activity").toString().toInt();
                int actor = reader.attributes().value("actor").toString().toInt();
                int sample = reader.attributes().value("sample").toString().toInt();

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

                InstanceInfo* instanceInfo = const_cast<InstanceInfo*>(dsMetaDataObject->instance(activity, actor, sample));

                if (!instanceInfo) {
                    instanceInfo = new InstanceInfo(dsMetaDataObject);
                    instanceInfo->setActivity(activity);
                    instanceInfo->setActor(actor);
                    instanceInfo->setSample(sample);
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
    InstanceInfo* instanceInfo = nullptr;
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
                int number = reader.attributes().value("size").toString().toInt();
                dsMetaDataObject->m_numberOfActivities = number;
                insideActivities = true;
            }
            // Activity tag
            else if (reader.name() == "activity" && insideActivities) {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                dsMetaDataObject->m_activities[key] = new QString(reader.text().toString());
            }
            // Actors tag
            else if (reader.name() == "actors") {
                int number = reader.attributes().value("size").toString().toInt();
                dsMetaDataObject->m_numberOfActors = number;
                insideActors = true;
            }
            // Actor tag
            else if (reader.name() == "actor" && insideActors) {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                dsMetaDataObject->m_actors[key] = new QString(reader.text().toString());
            }
            // Samples tag
            else if (reader.name() == "samples") {
                int number = reader.attributes().value("size").toString().toInt();
                dsMetaDataObject->m_numberOfSampleTypes = number;
                insideSamples = true;
            }
            // Sample tag
            else if (reader.name() == "sample" && insideSamples) {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                dsMetaDataObject->m_sampleTypes[key] = new QString(reader.text().toString());
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
                int activity = reader.attributes().value("activity").toString().toInt();
                int actor = reader.attributes().value("actor").toString().toInt();
                int sample = reader.attributes().value("sample").toString().toInt();

                instanceInfo = const_cast<InstanceInfo*>(dsMetaDataObject->instance(activity, actor, sample));

                if (instanceInfo) {
                    qDebug() << "Such an item should not exist yet";
                    throw 1;
                }

                instanceInfo = new InstanceInfo(dsMetaDataObject);
                instanceInfo->setActivity(activity);
                instanceInfo->setActor(actor);
                instanceInfo->setSample(sample);
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
        cerr << "Error parsing MSRDailyActivity3D dataset: " << endl;
        cerr << reader.errorString().toStdString() << endl;
    }

    reader.clear();
    file.close();

    return dsMetaDataObject;
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
