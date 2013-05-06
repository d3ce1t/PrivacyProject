#include "DatasetMetadata.h"
#include <QFile>
#include <QXmlStreamReader>
#include <iostream>

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


    QHashIterator<InstanceInfo::InstanceType, QHash<int, InstanceInfoList*>* > it(m_instances);

    while (it.hasNext()) {
        it.next();
        QHash<int, InstanceInfoList*>* activities = it.value();
        QHashIterator<int, InstanceInfoList*> it2(*activities);

        while (it2.hasNext()) {
            it2.next();
            InstanceInfoList* list = it2.value();

            for (int i=0; i<list->count(); ++i) {
                InstanceInfo* instance = list->at(i);
                delete instance;
            }

            list->clear(); // InstanceInfo stored still exists
            delete list;
        }

        activities->clear(); // Remove all pointers to InstanceInfoList
        delete activities;
    }

    m_instances.clear();

    // InstanceInfo will be deleted when DatasetMetadata is deleted
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

const InstanceInfo DatasetMetadata::instance(InstanceInfo::InstanceType type, int activity, int actor, int sample)
{
    InstanceInfo* result = 0;
    QHash<int, InstanceInfoList*>& hashInstances = *(m_instances[type]);
    InstanceInfoList* instancesList = hashInstances.value(activity);

    int i = 0;
    bool found = false;

    while (!found && i<instancesList->size())
    {
        InstanceInfo* instance = instancesList->at(i);

        if (instance->getActivity() == activity && instance->getActor() == actor && instance->getSample() == sample) {
            found = true;
            result = instance;
        }

        i++;
    }

    if (!found) {
        throw 1;
    }

    return *result;
}

const InstanceInfoList* DatasetMetadata::instances(
        InstanceInfo::InstanceType type, const QList<int>* activities, const QList<int>* actors, const QList<int>* samples) const
{
    InstanceInfoList* result = new InstanceInfoList();

    if (!m_instances.contains(type))
        return result;

    QHash<int, InstanceInfoList*>& hashInstances = *(m_instances[type]);

    for (int i=1; i<=m_numberOfActivities; ++i)
    {
        InstanceInfoList* instancesList = hashInstances.value(i); // instances with actors and samples

        for (int l=0; l<instancesList->size(); ++l)
        {
            InstanceInfo* instance = instancesList->at(l);
            bool isActivityChecked = false;
            bool isActorChecked = false;
            bool isSampleChecked = false;

            if (activities == 0 || (activities != 0 && activities->contains(instance->getActivity())))
                isActivityChecked = true;

            if (actors == 0 || (actors != 0 && actors->contains(instance->getActor())))
                isActorChecked = true;

            if (samples == 0 || (samples != 0 && samples->contains(instance->getSample())))
                isSampleChecked = true;

            if (isActivityChecked && isActorChecked && isSampleChecked) {
                result->append(instance);
            }
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

void DatasetMetadata::addInstanceInfo(InstanceInfo* instance)
{
    QHash<int, InstanceInfoList*>* activities = NULL;

    // Create list of activities if it doesn't exist
    // the list of activities contains a list of instances
    if (!m_instances.contains(instance->getType())) {
        activities = new QHash<int, InstanceInfoList*>();
        m_instances.insert(instance->getType(), activities);
    } else {
        activities = m_instances.value(instance->getType());
    }

    InstanceInfoList* list = NULL;

    if (!activities->contains(instance->getActivity())) {
        list = new InstanceInfoList();
        activities->insert(instance->getActivity(), list);
    } else {
        list = activities->value(instance->getActivity());
    }

    list->push_back(instance);
}

DatasetMetadata* DatasetMetadata::load(QString xmlPath)
{
    DatasetMetadata* result = new DatasetMetadata();
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
                result->m_name = reader.attributes().value("name").toString();
            }
            // Description tag
            else if (reader.name() == "description") {
                reader.readNext();
                result->m_description = reader.text().toString().trimmed();
            }
            // Path tag
            else if (reader.name() == "path") {
                reader.readNext();
                result->m_path = reader.text().toString().trimmed();
            }
            // Activities tag
            else if (reader.name() == "activities") {
                int number = reader.attributes().value("size").toString().toInt();
                result->m_numberOfActivities = number;
                insideActivities = true;
            }
            // Activity tag
            else if (reader.name() == "activity" && insideActivities) {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                result->m_activities[key] = new QString(reader.text().toString());
            }
            // Actors tag
            else if (reader.name() == "actors") {
                int number = reader.attributes().value("size").toString().toInt();
                result->m_numberOfActors = number;
                insideActors = true;
            }
            // Actor tag
            else if (reader.name() == "actor" && insideActors) {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                result->m_actors[key] = new QString(reader.text().toString());
            }
            // Samples tag
            else if (reader.name() == "samples") {
                int number = reader.attributes().value("size").toString().toInt();
                result->m_numberOfSampleTypes = number;
                insideSamples = true;
            }
            // Sample tag
            else if (reader.name() == "sample" && insideSamples) {
                int key = reader.attributes().value("key").toString().toInt();
                reader.readNext();
                result->m_sampleTypes[key] = new QString(reader.text().toString());
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

                InstanceInfo::InstanceType type;

                if (strType == "depth")
                    type = InstanceInfo::Depth;
                else if (strType == "color")
                    type = InstanceInfo::Color;
                else if (strType == "skeleton")
                    type = InstanceInfo::Skeleton;

                InstanceInfo* instanceInfo = new InstanceInfo(type);
                instanceInfo->setActivity(activity);
                instanceInfo->setActor(actor);
                instanceInfo->setSample(sample);
                instanceInfo->setFileName(file);
                instanceInfo->setDatasetPath(result->m_path);

                // Insert
                result->addInstanceInfo(instanceInfo);
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

    return result;
}
