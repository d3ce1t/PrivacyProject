#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <iostream>
#include "dataset/InstanceInfo.h"
#include "dataset/DatasetMetadata.h"
#include <QMultiMap>
#include "dataset/MSRAction3D/MSR3Action3D.h"

using namespace std;

dai::InstanceInfo* findInstance(QString activity, int actor, int sample, QMap<int, QMap<int, QMap<int, dai::InstanceInfo *> > > list);

void parseDAI4REID(const QString datasetPath)
{
    cout << "<dataset name=\"DAI4REID-Bin\">" << endl << endl;

    QDir datasetDir(datasetPath);
    QStringList filters;
    filters << "*_color.jpg";
    datasetDir.setNameFilters(filters);
    //datasetDir.setFilter(QDir::Files);

    // Parse Dataset
    struct DAISample {
        int actorId;
        int sampleId;
        int cameraId;
        QString type;
        QString file_color;
        QString file_depth;
        QString file_mask;
        QString file_skel;
    };

    QMap<int, QString> actors;
    QMap<int, QMap<int, QMap<int, DAISample> > > samples;
    int num_samples = 0;

    QStringList sampleEntries = datasetDir.entryList();

    for (auto it = sampleEntries.constBegin(); it != sampleEntries.constEnd(); ++it)
    {
        QString fileName = *it;

        int begin = 1, end = fileName.indexOf("_");
        DAISample sample;
        sample.actorId = fileName.mid(begin, end - begin).toInt();

        begin = end + 2;
        end = fileName.indexOf("_", begin);
        sample.cameraId = fileName.mid(begin, end - begin).toInt();

        begin = end + 2;
        end = fileName.indexOf("_", begin);
        sample.sampleId = fileName.mid(begin, end - begin).toInt();

        begin = end + 1;
        end = fileName.indexOf(".", begin);
        sample.type = fileName.mid(begin, end - begin);

        sample.file_color = fileName;
        sample.file_depth = QString(fileName).replace("_color.jpg", "_depth.bin");
        sample.file_mask = QString(fileName).replace("_color.jpg", "_mask.bin");
        sample.file_skel = QString(fileName).replace("_color.jpg", "_skel.bin");

        if (!QFile::exists(datasetPath + "/" + sample.file_depth) ||
                !QFile::exists(datasetPath + "/" + sample.file_mask) ||
                !QFile::exists(datasetPath + "/" + sample.file_skel)) {
            qDebug() << "Some missing files";
            throw 1;
        }

        samples[sample.actorId][sample.cameraId][sample.sampleId] = sample;
        actors.insert(sample.actorId, "Actor " + QString::number(sample.actorId) );
        num_samples++;
    }

    // Cameras
    cout << "\t" << "<cameras size=\"2\">" << endl;
    cout << "\t\t" << "<camera key=\"1\">Camera 1</camera>" << endl;
    cout << "\t\t" << "<camera key=\"2\">Camera 2</camera>" << endl;
    cout << "\t" << "</cameras>" << endl << endl;

    // Actors
    cout << "\t" << "<actors size=\"" << actors.size() << "\">" << endl;
    for (auto it = actors.constBegin(); it != actors.constEnd(); ++it) {
        cout << "\t\t" << "<actor key=\"" << it.key() << "\">" << it.value().toStdString() << "</actor>" << endl;
    }

    cout << "\t" << "</actors>" << endl << endl;

    // Samples
    int file_id = 1;
    cout << "\t" << "<instances>" << endl;

    for (auto cameras = samples.constBegin(); cameras != samples.constEnd(); ++cameras)
    {
        const QMap<int, QMap<int, DAISample>>& cameras_map = *cameras;

        for (auto frames = cameras_map.constBegin(); frames != cameras_map.constEnd(); ++frames)
        {
            const QMap<int, DAISample>& frames_map = *frames;

            foreach (DAISample sample, frames_map.values()) {
                cout << "\t\t" << "<instance actor=\"" << sample.actorId << "\" camera=\"" << sample.cameraId << "\" sample=\"" << sample.sampleId << "\">" << endl;
                cout << "\t\t\t" << "<file id=\"" << file_id << "\">" << sample.file_color.toStdString() << "</file>" << endl;
                cout << "\t\t\t" << "<file id=\"" << file_id+1 << "\">" << sample.file_depth.toStdString() << "</file>" << endl;
                cout << "\t\t\t" << "<file id=\"" << file_id+2 << "\">" << sample.file_mask.toStdString() << "</file>" << endl;
                cout << "\t\t\t" << "<file id=\"" << file_id+3 << "\">" << sample.file_skel.toStdString() << "</file>" << endl;
                cout << "\t\t\t" << "<data type=\"color\" file-ref=\"" << file_id << "\" />" << endl;
                cout << "\t\t\t" << "<data type=\"depth\" file-ref=\"" << file_id+1 << "\" />" << endl;
                cout << "\t\t\t" << "<data type=\"mask\" file-ref=\"" << file_id+2 << "\" />" << endl;
                cout << "\t\t\t" << "<data type=\"skeleton\" file-ref=\"" << file_id+3 << "\" />" << endl;
                cout << "\t\t" << "</instance>" << endl;
                file_id+=4;
            }
        }
    }

    cout << "\t" << "</instances>" << endl;

    cout << "</dataset>" << endl;
}

void parseCAVIAR4REID(const QString datasetPath)
{
    cout << "<dataset name=\"CAVIAR4REID\">" << endl << endl;

    QDir datasetDir(datasetPath);
    datasetDir.setFilter(QDir::Files);

    // Parse Dataset
    struct CaviarSample {
        int actorId;
        int sampleId;
        int cameraId;
        QString file;
    };

    QMap<int, QString> actors;
    QList<CaviarSample> samples;
    int num_samples = 0;


    QStringList sampleEntries = datasetDir.entryList();

    for (auto it = sampleEntries.constBegin(); it != sampleEntries.constEnd(); ++it)
    {
        QString fileName = *it;
        CaviarSample sample;
        sample.actorId = fileName.mid(0, 4).toInt();
        sample.sampleId = fileName.mid(4, 3).toInt();
        sample.cameraId = (sample.sampleId - 1) / 10 + 1;
        sample.file = fileName;
        samples << sample;
        actors.insert(sample.actorId, "Actor " + QString::number(sample.actorId) );
        num_samples++;
    }

    // Cameras
    cout << "\t" << "<cameras size=\"2\">" << endl;
    cout << "\t\t" << "<camera key=\"1\">Camera 1</camera>" << endl;
    cout << "\t\t" << "<camera key=\"2\">Camera 2</camera>" << endl;
    cout << "\t" << "</cameras>" << endl << endl;

    // Actors
    cout << "\t" << "<actors size=\"" << actors.size() << "\">" << endl;
    for (auto it = actors.constBegin(); it != actors.constEnd(); ++it) {
        cout << "\t\t" << "<actor key=\"" << it.key() << "\">" << it.value().toStdString() << "</actor>" << endl;
    }

    cout << "\t" << "</actors>" << endl;

    // Samples
    int file_id = 1;
    cout << "\t" << "<instances>" << endl;
    foreach (CaviarSample sample, samples) {
        cout << "\t\t" << "<instance actor=\"" << sample.actorId << "\" camera=\"" << sample.cameraId << "\" sample=\"" << sample.sampleId << "\">" << endl;
        cout << "\t\t\t" << "<file id=\"" << file_id << "\">" << sample.file.toStdString() << "</file>" << endl;
        cout << "\t\t\t" << "<data type=\"color\" file-ref=\"" << file_id << "\" />" << endl;
        cout << "\t\t" << "</instance>" << endl;
        file_id++;
    }

    cout << "\t" << "</instances>" << endl;

    cout << "</dataset>" << endl;
}

void parseETHC(const QString datasetPath)
{
    QDir datasetDir(datasetPath);
    datasetDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList seqEntries = datasetDir.entryList();

    int num_samples = 0;

    /*cout << "<dataset name=\"ETHZ\">" << endl << endl;
    cout << "\t" << "<activities size=\"3\">" << endl;
    cout << "\t\t" << "<activity key=\"1\">Sequence 1</activity>" << endl;
    cout << "\t\t" << "<activity key=\"2\">Sequence 2</activity>" << endl;
    cout << "\t\t" << "<activity key=\"3\">Sequence 3</activity>" << endl;
    cout << "\t" << "</activities>" << endl << endl;*/

    QMultiMap<int, QString> actors;

    for (QStringList::ConstIterator seq_it = seqEntries.constBegin(); seq_it != seqEntries.constEnd(); ++seq_it)
    {
        QDir personsDir = datasetDir;
        personsDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        personsDir.cd(*seq_it);

        QStringList personEntries = personsDir.entryList();

        for (QStringList::ConstIterator person_it = personEntries.constBegin(); person_it != personEntries.constEnd(); ++person_it)
        {
            QDir personSamples = personsDir;
            personSamples.setFilter(QDir::Files);
            personSamples.cd(*person_it);

            QStringList sampleEntries = personSamples.entryList();
            QString actor = (*seq_it) + "#" + (*person_it);
            actors.insert(sampleEntries.size(), actor);


            /*for (QStringList::ConstIterator sample_it = sampleEntries.constBegin(); sample_it != sampleEntries.constEnd(); ++sample_it)
            {
                num_samples++;
                //cout << (*seq_it).toStdString() << " " << (*person_it).toStdString() << " " << (*sample_it).toStdString() << endl;
            }*/
        }
    }

    auto it = actors.constEnd();
    const int max_actors = 10;
    int i = 0;

    while (it != actors.constBegin() && i < max_actors)
    {
        --it;
        qDebug() << it.value() << it.key();
        ++i;
    }

    //cout << "</dataset>" << endl;
    //cout << "Num.Samples" << num_samples;
}

void parseMSRDailyActivity3D(const QStringList& entries)
{   
    for( QStringList::ConstIterator entry=entries.begin(); entry!=entries.end(); ++entry )
    {
        // a01_s01_e01_depth.bin
        QString fileEntry = *entry;
        QString activity = fileEntry.mid(1, 2);
        QString subject = fileEntry.mid(5, 2);
        QString extension = fileEntry.mid(fileEntry.indexOf(".")+1);
        QString sample = fileEntry.mid(fileEntry.indexOf("_e")+2, 2);

        QString type = "unknown";

        if (extension == "bin") {
            type = "depth";
        } else if (extension == "avi") {
            type = "color";
        } else if (extension == "txt") {
            type = "skeleton";
        }

        cout << "<instance " <<
                "type=\"" << type.toStdString() << "\" " <<
                "activity=\"" << activity.toStdString() << "\" " <<
                "actor=\"" << subject.toStdString() << "\" " <<
                "sample=\"" << sample.toStdString()<< "\">" << endl;
        cout << "\t<file>" << fileEntry.toStdString() << "</file>" << endl;
        cout << "</instance>" << endl;
    }
}

void parseMSRAction3D(const QString datasetPath)
{
    QDir datasetDir(datasetPath);
    QStringList filters;
    filters << "*.bin";
    datasetDir.setNameFilters(filters);
    QStringList entries = datasetDir.entryList();

    cout << "<dataset name=\"MSRAction3D\">" << endl;

    cout << "\t" << "<description>20 action types, 10 subjects, each subject performs each action 2 or 3 times. There are 567 depth map sequences "
         << "in total. The resolution is 320x240. The data was recorded with a depth sensor similar to the Kinect device.</description>" << endl;

    // Cameras
    cout << "\t" << "<cameras size=\"1\">" << endl;
    cout << "\t\t" << "<camera key=\"1\">Camera 1</camera>" << endl;
    cout << "\t" << "</cameras>" << endl << endl;

    // Actors
    cout << "\t" << "<actors size=\"10\">" << endl;
    cout << "\t\t" << "<actor key=\"1\">Actor 1</actor>" << endl;
    cout << "\t\t" << "<actor key=\"2\">Actor 2</actor>" << endl;
    cout << "\t\t" << "<actor key=\"3\">Actor 3</actor>" << endl;
    cout << "\t\t" << "<actor key=\"4\">Actor 4</actor>" << endl;
    cout << "\t\t" << "<actor key=\"5\">Actor 5</actor>" << endl;
    cout << "\t\t" << "<actor key=\"6\">Actor 6</actor>" << endl;
    cout << "\t\t" << "<actor key=\"7\">Actor 7</actor>" << endl;
    cout << "\t\t" << "<actor key=\"8\">Actor 8</actor>" << endl;
    cout << "\t\t" << "<actor key=\"9\">Actor 9</actor>" << endl;
    cout << "\t\t" << "<actor key=\"10\">Actor 10</actor>" << endl;
    cout << "\t" << "</actors>" << endl << endl;

    // Labels for activities
    cout << "\t" << "<labels name=\"activities\" size=\"20\">" << endl;
    cout << "\t\t" << "<label key=\"act1\">High arm wave</label>" << endl;
    cout << "\t\t" << "<label key=\"act2\">Horizontal arm wave</label>" << endl;
    cout << "\t\t" << "<label key=\"act3\">Hammer</label>" << endl;
    cout << "\t\t" << "<label key=\"act4\">Hand catch</label>" << endl;
    cout << "\t\t" << "<label key=\"act5\">Forward punch</label>" << endl;
    cout << "\t\t" << "<label key=\"act6\">High throw</label>" << endl;
    cout << "\t\t" << "<label key=\"act7\">Draw x</label>" << endl;
    cout << "\t\t" << "<label key=\"act8\">Draw tick</label>" << endl;
    cout << "\t\t" << "<label key=\"act9\">Draw circle</label>" << endl;
    cout << "\t\t" << "<label key=\"act10\">Hand clap</label>" << endl;
    cout << "\t\t" << "<label key=\"act11\">Two hand wave</label>" << endl;
    cout << "\t\t" << "<label key=\"act12\">Side-boxing</label>" << endl;
    cout << "\t\t" << "<label key=\"act13\">Bend</label>" << endl;
    cout << "\t\t" << "<label key=\"act14\">Forward kick</label>" << endl;
    cout << "\t\t" << "<label key=\"act15\">Side kick</label>" << endl;
    cout << "\t\t" << "<label key=\"act16\">Jogging</label>" << endl;
    cout << "\t\t" << "<label key=\"act17\">Tennis swing</label>" << endl;
    cout << "\t\t" << "<label key=\"act18\">Tennis serve</label>" << endl;
    cout << "\t\t" << "<label key=\"act19\">Gold swing</label>" << endl;
    cout << "\t\t" << "<label key=\"act20\">Pickup and throw</label>" << endl;
    cout << "\t" << "</labels>" << endl;

    // Labels for repetitions
    cout << "\t" << "<labels name=\"repetition\" size=\"3\">" << endl;
    cout << "\t\t" << "<label key=\"rep1\">First repetition</label>" << endl;
    cout << "\t\t" << "<label key=\"rep2\">Second repetition</label>" << endl;
    cout << "\t\t" << "<label key=\"rep3\">Third repetition</label>" << endl;
    cout << "\t" << "</labels>" << endl;

    // Instances
    cout << "\t" << "<instances>" << endl;

    int file_id = 1;

    for( QStringList::ConstIterator entry=entries.begin(); entry!=entries.end(); ++entry )
    {
        // a01_s01_e01_sdepth.bin
        QString fileEntry = *entry;
        int activity = fileEntry.mid(1, 2).toInt();
        int subject = fileEntry.mid(5, 2).toInt();
        int sample = fileEntry.mid(fileEntry.indexOf("_e")+2, 2).toInt();
        QString strAct = "act" + QString::number(activity);
        QString repetition = "rep" + QString::number(sample);


        cout << "\t\t" << "<instance actor=\"" << subject << "\" camera=\"1\" sample=\"" << sample << "\" label=\"" << strAct.toStdString() << "," << repetition.toStdString() << "\">" << endl;
        cout << "\t\t\t" << "<file id=\"" << file_id << "\">" << fileEntry.toStdString() << "</file>" << endl;

        QString skelFileName = fileEntry.replace("sdepth.bin", "skeleton3D.txt");
        QFile skelFile(datasetPath + "/" + skelFileName);
        bool skelExist = false;

        if (skelFile.exists()) {
            cout << "\t\t\t" << "<file id=\"" << file_id+1 << "\">" << skelFileName.toStdString() << "</file>" << endl;
            skelExist = true;
        }

        cout << "\t\t\t" << "<data type=\"depth\" file-ref=\"" << file_id << "\" />" << endl;
        if (skelExist)
                cout << "\t\t\t" << "<data type=\"skeleton\" file-ref=\"" << file_id+1 << "\" />" << endl;
        //        "activity=\"" << activity.toStdString() << "\" " <<

        cout << "\t\t" << "</instance>" << endl;
        file_id+=2;
    }

    cout << "\t" << "</instances>" << endl;

    cout << "</dataset>" << endl;
}

void parseMSRAction3D_Quaternions(const QString datasetPath)
{
    dai::Dataset* dataset = new dai::MSR3Action3D;
    dataset.setPath(datasetPath);
    const dai::DatasetMetadata& metadata = dataset->getMetadata();

    metadata.instances()



    delete dataset;
}

void parseHuDaAct(const QStringList& entries)
{
    QHash<QString, int> map;
    map["L"] = 1; // L: enter room
    map["E"] = 2; // E: exit the room
    map["P"] =   3; // P: make a phone call
    map["B"] =   4; // B: go to bed
    map["G"] =   5; // G: get up from bed
    map["I"] =  6; // I: sit down
    map["T"] =  7; // T: stand up
    map["O"] =  8; // O: mop floor
    map["M"] =   9; // M: eat meal
    map["D"] = 10; // D: put on jacket
    map["N"] = 11; // N: take off jacket
    map["K"] = 12; // K: drink water
    map["BG"] =13; // BG: background activity
    map["SBG"] =14; // SBG: background activity
    map["CB"] =15; // CB: background activity
    map["SCB"] = 16;  // SCB: background activity


    QMap<int, QMap<int, QMap<int, dai::InstanceInfo*> > > list;

    for( QStringList::ConstIterator entry=entries.begin(); entry!=entries.end(); ++entry )
    {
        // S5_C4_U10_BG.oni, S5_C4_U10_E2.oni
        int start = 0, end = 0;
        QString fileEntry = *entry;

        start = fileEntry.indexOf(".") + 1;
        QString extension = fileEntry.mid(start);

        if (extension != "oni")
            continue;

        start = fileEntry.indexOf("S") + 1;
        end = fileEntry.indexOf("_", start);
        QString session = fileEntry.mid(start, end-start);

        start = end + 2;
        end = fileEntry.indexOf("_", start);
        QString calibration = fileEntry.mid(start, end-start);

        start = end + 2;
        end = fileEntry.indexOf("_", start);
        int subject = fileEntry.mid(start, end-start).toInt();

        start = end + 1;
        end = fileEntry.indexOf(".", start);
        QString activity = fileEntry.mid(start, end-start);

        int sample = 1;

        if (activity.length() == 2 && activity[1].isNumber()) {
            sample = QString(activity[1]).toInt();
            activity = activity[0];
        }

        // Build Object Model
        dai::InstanceInfo* instanceInfo = findInstance(QString::number(map[activity]), subject, sample, list);

        if (!instanceInfo) {
            instanceInfo = new dai::InstanceInfo;
            instanceInfo->addLabel(QString::number(map[activity]));
            instanceInfo->setCamera(1);
            instanceInfo->setActor(subject);
            instanceInfo->setSample(sample);
            list[map[activity]][subject][sample] = instanceInfo;
            //list.insertMulti(map[activity], instanceInfo);
        }

        instanceInfo->addType(dai::DataFrame::Color);
        instanceInfo->addType(dai::DataFrame::Depth);
        instanceInfo->addType(dai::DataFrame::Mask);
        instanceInfo->addType(dai::DataFrame::Skeleton);
        instanceInfo->addFileName(dai::DataFrame::Color, fileEntry);
        instanceInfo->addFileName(dai::DataFrame::Depth, fileEntry);
        instanceInfo->addFileName(dai::DataFrame::Mask, fileEntry);
        instanceInfo->addFileName(dai::DataFrame::Skeleton, fileEntry);
    }

    int fileId = 1;

    foreach (int actKey, list.keys())
    {
        QMap<int, QMap<int, dai::InstanceInfo*>> actList = list.value(actKey);

        foreach (int subKey, actList.keys())
        {
            QMap<int, dai::InstanceInfo*> subList = actList.value(subKey);

            foreach (dai::InstanceInfo* info, subList.values())
            {
               /*
                * Output:
                * <instance activity="D" actor="U10" sample="1">
                *      <file id="3">S5_C4_U10_D1.oni</file>
                *      <data type="depth" file-id="3" />
                *      <data type="skeleton" file-id="3" />
                *      <data type="color" file-id="3" />
                * </instance>
                */

                cout << "<instance " <<
                        "activity=\"" << info->getLabels().at(0).toStdString() << "\" " <<
                        "actor=\"" << info->getActor() << "\" " <<
                        "sample=\"" << info->getSample() << "\">" << endl;

                QString num;
                num.setNum(fileId);

                cout << "\t<file id=\"" << num.toStdString() << "\">" << info->getFileName(dai::DataFrame::Color).toStdString() << "</file>" << endl;
                cout << "\t<data type=\"color\" file-ref=\"" << num.toStdString() << "\" />" << endl;
                cout << "\t<data type=\"depth\" file-ref=\"" << num.toStdString() << "\" />" << endl;
                cout << "\t<data type=\"skeleton\" file-ref=\"" << num.toStdString() << "\" />" << endl;
                cout << "\t<data type=\"mask\" file-ref=\"" << num.toStdString() << "\" />" << endl;

                cout << "</instance>" << endl;

                fileId++;
            }
        }
    }
}

dai::InstanceInfo* findInstance(QString activity, int actor, int sample, QMap<int, QMap<int, QMap<int, dai::InstanceInfo*> > > list)
{
    using namespace dai;

    InstanceInfo* result = nullptr;
    bool found = false;

    QMapIterator<int, QMap<int, QMap<int, dai::InstanceInfo*>>> it(list);

    while (!found && it.hasNext())
    {
        it.next();
        QMap<int, QMap<int, dai::InstanceInfo*>> actList = it.value();
        QMapIterator<int, QMap<int, dai::InstanceInfo*>> it2(actList);

        while (!found && it2.hasNext())
        {
            it2.next();
            QMap<int, dai::InstanceInfo*> subList = it2.value();
            QMapIterator<int, dai::InstanceInfo*> it3(subList);

            while (!found && it3.hasNext())
            {
                it3.next();
                InstanceInfo* instance = it3.value();

                if (instance->hasLabels({activity}) && instance->getActor() == actor && instance->getSample() == sample) {
                    found = true;
                    result = instance;
                }
            }
        }
    }

    return result;
}



int main(int argc, char *argv[])
{
    //parseMSRAction3D(argv[1]);
    //parseCAVIAR4REID(argv[1]);
    //parseDAI4REID(argv[1]);
    parseMSRAction3D_Quaternions(argv[1]);
}

