#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <iostream>
#include "dataset/InstanceInfo.h"
#include "dataset/DatasetMetadata.h"
#include "dataset/DAI4REID/DAI4REID.h"
#include <QMultiMap>
#include "dataset/MSRAction3D/MSR3Action3D.h"
#include "types/SkeletonFrame.h"
#include "types/ColorFrame.h"
#include "types/MaskFrame.h"
#include "types/DepthFrame.h"
#include "types/MetadataFrame.h"
#include "openni/OpenNIColorInstance.h"
#include "opencv_utils.h"
#include <QImage>
#include "Config.h"

using namespace std;

dai::InstanceInfo* findInstance(QString activity, int actor, int sample, QMap<int, QMap<int, QMap<int, dai::InstanceInfo *> > > list);

struct DatasetSample {
    int actorId;
    int sampleId;
    int cameraId;
    QString file_color;
    QString file_depth;
    QString file_mask;
    QString file_skel;
};

void writeXML(QMap<int, QMap<int, QMap<int, DatasetSample> > > samples)
{
    int file_id = 1;

    cout << "\t" << "<instances>" << endl;

    for (auto cameras = samples.constBegin(); cameras != samples.constEnd(); ++cameras)
    {
        const QMap<int, QMap<int, DatasetSample>>& cameras_map = *cameras;

        for (auto frames = cameras_map.constBegin(); frames != cameras_map.constEnd(); ++frames)
        {
            const QMap<int, DatasetSample>& frames_map = *frames;

            foreach (DatasetSample sample, frames_map.values()) {
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
}

void parseIAS_LAB_RGBD_ID(const QString datasetPath)
{
    QList<QString> actor_folders = {"000", "001", "002", "003", "004", "005", "006", "007", "008", "009", "010"};
    QMap<int, QMap<int, QMap<int, DatasetSample> > > samples;
    int num_samples = 0;

    // Lambda function -> Parse Dataset
    auto parseDataset = [datasetPath, actor_folders, &samples, &num_samples](QString subfolder, int camera)
    {
        for (QString actor_folder : actor_folders)
        {
            QDir datasetDir(datasetPath + "/" + subfolder + "/" + actor_folder);
            QStringList filters;
            filters << "*_rgb.jpg";
            datasetDir.setNameFilters(filters);

            QStringList sampleEntries = datasetDir.entryList();

            for (QString fileName : sampleEntries)
            {
                DatasetSample sample;
                sample.actorId = actor_folder.toInt() + 1;
                sample.cameraId = camera;
                sample.sampleId = ++num_samples;

                sample.file_color = subfolder + "/" + actor_folder + "/" + fileName;
                sample.file_depth = subfolder + "/" + actor_folder + "/" + QString(fileName).replace("_rgb.jpg", "_depth.pgm");
                sample.file_mask  = subfolder + "/" + actor_folder + "/" + QString(fileName).replace("_rgb.jpg", "_userMap.pgm");
                sample.file_skel  = subfolder + "/" + actor_folder + "/" + QString(fileName).replace("_rgb.jpg", "_skel.txt");

                if (!QFile::exists(datasetPath + "/" + sample.file_depth) ||
                        !QFile::exists(datasetPath + "/" + sample.file_mask) ||
                        !QFile::exists(datasetPath + "/" + sample.file_skel)) {
                    qDebug() << "Some missing files";
                    throw 1;
                }

                samples[sample.actorId][sample.cameraId][sample.sampleId] = sample;
            }
        }
    };

    parseDataset("Training", 1);
    parseDataset("TestingB", 2);

    cout << "<dataset name=\"IAS-LAB-RGBD-ID\">" << endl << endl;

    // Cameras
    cout << "\t" << "<cameras size=\"2\">" << endl;
    cout << "\t\t" << "<camera key=\"1\">Camera 1</camera>" << endl;
    cout << "\t\t" << "<camera key=\"2\">Camera 2</camera>" << endl;
    cout << "\t" << "</cameras>" << endl << endl;

    // Actors
    int i = 1;
    cout << "\t" << "<actors size=\"" << actor_folders.size() << "\">" << endl;

    for (QString actor_folder : actor_folders)  {
        cout << "\t\t" << "<actor key=\"" << i++ << "\">" << "Actor " << actor_folder.toStdString() << "</actor>" << endl;
    }

    cout << "\t" << "</actors>" << endl << endl;

    // Samples
    writeXML(samples);

    cout << "</dataset>" << endl;
}


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

void parseGeorgeFeatures_Sample()
{
    QString long_str = "-0.35885 0.3639 2.7042 -0.004064 0.36632 2.6564 -0.19438 0.4108 2.5867 -0.16748 0.20959 2.7761 -0.31497 -0.006219 2.67 -0.03664 -0.024287 2.6339 -0.15886 -0.000245 2.775 -0.44069 0.11475 2.7613 0.15705 0.18282 2.545 -0.52374 -0.1575 2.8161 0.38695 0.039984 2.4413 -0.51265 -0.22357 2.8029 0.44926 0.01885 2.423 -0.32694 -0.57874 2.714 -0.069307 -0.59161 2.6957 -0.34793 -1.0002 2.7792 -0.057811 -1.0167 2.7342 -0.37771 -1.0241 2.6152 -0.078119 -1.0323 2.5678 -0.18657 0.66442 2.5568 0.358 0.20749 0.25615 0.37427 0.50936 0.42144 0.2684 0.56946 0.55818 0.85455 0.61525 0.92258 0.94323 0.99845 1.3662 1.4134 1.391 1.4307 0.37644 0.20749 0.25615 0.48542 0.39261 0.41521 0.51473 0.2684 0.75495 0.55286 0.79254 0.617 1.0004 0.96095 1.4145 1.3863 1.4403 1.4034 0.36343 0.27765 0.44202 0.46519 0.45351 0.42286 0.42097 0.69573 0.7047 0.74194 0.77115 1.0065 1.016 1.4323 1.4416 1.4468 1.4479 0.2555 0.28212 0.3034 0.21001 0.28958 0.39933 0.5131 0.66955 0.55452 0.73582 0.80669 0.81119 1.2232 1.2319 1.2618 1.2624 0.50531 0.28125 0.18822 0.19692 0.52362 0.29631 0.7397 0.32247 0.80355 0.57433 0.63537 1.0005 1.0447 1.0213 1.058 0.69214 0.18822 0.4459 0.29717 0.53684 0.46975 0.54302 0.53144 0.63096 0.57161 1.0346 0.99771 1.0565 1.0111 0.70904 0.30469 0.43153 0.39944 0.64102 0.41931 0.70289 0.6055 0.60334 1.0177 1.0223 1.0591 1.0558 0.70011 0.63932 0.28985 0.89052 0.34839 0.9569 0.70435 0.80074 1.119 1.1948 1.1499 1.2185 0.63917 0.80795 0.28985 0.82474 0.35657 0.91803 0.82079 1.3074 1.2332 1.3219 1.2379 0.59174 1.0044 0.068276 1.0641 0.47601 0.63988 0.86165 0.98084 0.90148 1.0127 0.92544 1.0048 0.068276 0.98329 0.81965 1.3177 1.1833 1.3218 1.1756 0.85569 1.0622 0.41053 0.58609 0.79427 0.91689 0.83324 0.94774 0.97746 1.0219 0.84612 1.342 1.1943 1.3448 1.1849 0.91594 0.25861 0.427 0.51446 0.459 0.53763 1.2609 0.50155 0.427 0.53724 0.459 1.2691 0.29405 0.16837 0.34427 1.6872 0.34139 0.16837 1.6954 0.30343 1.7003 1.7002 166.41 164.06 123.9 102.95 108.35 119.24 124.9 178.15 145.71 61.753 61.753 136.41 99.111 90.493 175.07 90 90.041 90.356 175.32 90 33.807 31.544 98.514 89.159 24.705 16.497 0.062306 -0.021134 -0.018248 0.2299 -0.14283 -0.10371 0.16111 -0.1835 -0.11139 0.19031 -0.044476 0.069665 -0.007807 -0.25363 0.029884 0.011093 -0.066077 -0.01313 -0.083052 -0.27225 0.054735 -0.081838 -0.24915 0.057159 -0.16447 -0.046897 0.11748 -0.026901 0.20121 -0.18943 -0.008616 0.20983 0.001143 -0.12222 0.024042 0.1411 0.032667 0.56732 -0.061811 -0.011496 0.42511 -0.038477 0.020308 0.015611 0.16642 0.15611 0.005974 0.10497 0.01197 0.57252 -0.043983 0.020989 0.42148 -0.065157 0.029772 0.02387 0.16399 0.11835 0.088487 -0.4839 0.86256 0.13868 0.14466 -0.41244 0.88867 0.47022 0.33869 0.61909 -0.53001 0.62289 -0.24742 0.094353 0.73614 0.58532 -0.45753 -0.064422 -0.66628 0.5057 -0.044962 -0.77661 -0.37298 0.46256 -0.51608 0.0041329 -0.72089 0.016131 -0.76676 0.10669 -0.6328 0.29483 -0.61627 0.041417 -0.72909 0.85828 0.056546 -0.34561 -0.37512 0.85828 0.14918 0.34702 0.3474 0.37128 -0.92332 -0.038404 0.090328 0.64871 -0.57679 -0.020982 -0.49603 0.70406 0.53821 0.019441 0.46287 0.043018 -0.21179 -0.093692 -0.97186 0.70711 -0.70169 -0.011131 0.086672 0.70685 -0.39404 -0.019378 0.58712 0.70491 0.39607 -0.0533 -0.586 0.04085 0.9366 0.0071486 0.34795 0.70711 -0.6951 0.052932 0.11849 0.9568 0.091956 -0.035068 -0.2736 0.96235 0.15338 0.021596 0.22336 0.65267 -0.2245 -0.21989 -0.6894 0.71227 -0.23139 -0.10684 -0.654 0.97685 -0.013736 0.026326 -0.21185 0.98966 -0.021335 0.026322 -0.13941 195.61 6.8076 120.48 202 7.3056 128.79 109.23 355.46 77.912 101.91 60.502 245.24 278.62 313.86 243.75 239.39 342.1 318.96 266.95 317.84 219.49 280.34 349.4 173.56 265.64 331.25 200.22 311.05 316.92 347.17 43.538 44.416 1.2015 8.026 7.9317 223.25 310.23 321.73 258.43 320.48 42.277 90.607 204.84 357.48 191.57 6.1377 7.9433 270.01 54.56 57.703 270 51.675 299.73 90.062 319.33 2.3967 175.89 13.924 5.3926 270.32 358.87 327.99 10.654 358.28 25.882 18.506 265.47 306.76 265.08 284.98 298.09 274.57 2.8731 335.5 359.01 2.753 343.92 357.92 0 4.6071e-16 0.079593 0.053776 0.122 0.1258 0 0.019508 0.13324 0.047415 0.23994 0.075518 0.2549 0.22811 0.21613 0.27637 0.28675 0.44341 0.45279 0.042232 0.104063621105483 0.0415830101470132 0.0110546305136231 0.00655882257019247 0.00444210343122973 0.00131657086220442 0.00156749563103369 0.107515732848432 0.104586997130892 0.080192512310883 0.0668525039229112 0.0462441386583774 0.0452571497749412 0.0316243348460039 0.0152674670280326 0.034067171168904 0.0328262883911501 0.0227827811390078 0.0157015863497795 0.00691055843524658 0.0106397786836064 0.0162670665897861 0.0850757505673015 0.107601927893965";
    QStringList list = long_str.split(" ");

    // Skeleton Poses (60)
    qDebug() << "Positions:";
    int c = 0;
    for (int i=0; i<60; i+=3) {
        qDebug() << c++ << list.value(i) << list.value(i+1) << list.value(i+2);
    }

    // Distances (190)
    QString distances;
    for (int i=60; i<250; ++i) {
        distances += list.value(i) + " ";
    }
    qDebug() << "Distances" << distances.trimmed();

    // Angles (26)
    QString angles;
    for (int i=250; i<276; ++i) {
        angles += list.value(i) + " ";
    }
    qDebug() << "Angles" << angles.trimmed();

    // Position diferences (57)
    qDebug() << "Positions diff.:";
    c = 0;
    for (int i=276; i<333; i+=3) {
        qDebug() << c++ << list.value(i) << list.value(i+1) << list.value(i+2);
    }

    // Quaternions (104)
    qDebug() << "Quaternions:";
    c = 0;
    for (int i=333; i<437; i+=4) {
        qDebug() << c++ << list.value(i) << list.value(i+1) << list.value(i+2) << list.value(i+3);
    }

    // Euler Angles (78)
    qDebug() << "Euler Angles:";
    c = 0;
    for (int i=437; i<515; i+=3) {
        qDebug() << c++ << list.value(i) << list.value(i+1) << list.value(i+2);
    }

    // Plane distances (20)
    QString plane;
    for (int i=515; i<535; ++i) {
        plane += list.value(i) + " ";
    }
    qDebug() << "Plane Distances" << plane.trimmed();

    qDebug() << list.size();
}

void parseMSRAction3D_Quaternions(const QString datasetPath)
{
    shared_ptr<dai::Dataset> dataset = static_pointer_cast<dai::Dataset>(make_shared<dai::MSR3Action3D>());
    dataset->setPath(datasetPath);
    const dai::DatasetMetadata& metadata = dataset->getMetadata();

    // Get all instances of the MSRAction-3D dataset
    //QList<shared_ptr<dai::InstanceInfo>> instances_info = metadata.instances({1},{1}, {{"act2","rep1"}});
    QList<shared_ptr<dai::InstanceInfo>> instances_info = metadata.instances();

    // Container for read frames
    dai::QHashDataFrames readFrames;
    readFrames.insert( dai::DataFrame::Skeleton, make_shared<dai::SkeletonFrame>() );

    for (auto info : instances_info)
    {
        // Get Sample
        shared_ptr<dai::StreamInstance> instance_tmp = dataset->getInstance(*info, dai::DataFrame::Skeleton);
        shared_ptr<dai::DataInstance> instance = static_pointer_cast<dai::DataInstance>(instance_tmp);

        // Open Instance
        QString fileName = datasetPath + "/" + info->getFileName(dai::DataFrame::Skeleton);
        QString outputFile = fileName.replace("skeleton3D", "quaternion");
        //QString outputFile = fileName.replace("skeleton3D", "eulerangles");
        QFile of(outputFile);

        instance->open();

        if (!of.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Error opening file" << outputFile;
            return;
        }

        QTextStream in(&of);

        // Traverse
        while (instance->hasNext()) {

            // Read frames
            instance->readNextFrame(readFrames);

            // Get skeleton frame and first skeleton of the frame
            shared_ptr<dai::SkeletonFrame> skeletonFrame = static_pointer_cast<dai::SkeletonFrame>(readFrames.value(dai::DataFrame::Skeleton));
            shared_ptr<dai::Skeleton> skeleton = skeletonFrame->getSkeleton(1);

            for (dai::Quaternion q : skeleton->quaternions())
            {
                //dai::Vector3d eulerAngles = q.toEulerAngles();
                //in << eulerAngles.x() << " " << eulerAngles.y() << " " << eulerAngles.z() << " ";
                in << q.toString() << " ";
            }

            in << endl;
        }

        // Close instance
        of.close();
        instance->close();
    }

    qDebug() << "Number of instances:" << instances_info.size();
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

void convertDAI4REIDOniToFiles()
{
    using namespace dai;
    using namespace cv;

    Dataset* dataset = new DAI4REID;
    dataset->setPath("/files/DAI4REID");
    const DatasetMetadata& metadata = dataset->getMetadata();

    // Create memory for all kind of frames
    QHashDataFrames readFrames;
    readFrames.insert(DataFrame::Color, make_shared<ColorFrame>(640, 480));
    readFrames.insert(DataFrame::Depth, make_shared<DepthFrame>(640, 480));
    readFrames.insert(DataFrame::Mask, make_shared<MaskFrame>(640, 480));
    readFrames.insert(DataFrame::Skeleton, make_shared<SkeletonFrame>());
    readFrames.insert(DataFrame::Metadata, make_shared<MetadataFrame>());

    // For each actor, parse his/her samples
    QList<int> actors = {1,2,3,4,5};

    for (int actor : actors)
    {
        QList<shared_ptr<InstanceInfo>> instances_md = metadata.instances({actor},
                                                                       {2},
                                                                       DatasetMetadata::ANY_LABEL);
        shared_ptr<InstanceInfo> instance_info = instances_md.at(0);
        std::string fileName = instance_info->getFileName(DataFrame::Color).toStdString();

        printf("actor %i sample %i file %s\n", instance_info->getActor(),
               instance_info->getSample(),
               fileName.c_str());

        std::fflush(stdout);

        // Get instances
        QList<shared_ptr<StreamInstance>> instances;
        instances << dataset->getInstance(*instance_info, DataFrame::Color);
        instances << dataset->getInstance(*instance_info, DataFrame::Metadata);

        // Open Instances
        for (shared_ptr<StreamInstance> instance : instances) {
            instance->open();
        }

        shared_ptr<OpenNIColorInstance> colorInstance = static_pointer_cast<OpenNIColorInstance>(instances.at(0));
        colorInstance->device().playbackControl()->setSpeed(0.15f);

        // Read frames
        uint previousFrame = 0;

        while (colorInstance->hasNext())
        {
            for (shared_ptr<StreamInstance> instance : instances) {
                instance->readNextFrame(readFrames);
            }

            // Get Frames
            auto colorFrame = static_pointer_cast<ColorFrame>(readFrames.value(DataFrame::Color));
            auto depthFrame = static_pointer_cast<DepthFrame>(readFrames.value(DataFrame::Depth));
            auto maskFrame = static_pointer_cast<MaskFrame>(readFrames.value(DataFrame::Mask));
            auto skeletonFrame = static_pointer_cast<SkeletonFrame>(readFrames.value(DataFrame::Skeleton));
            auto metadataFrame = static_pointer_cast<MetadataFrame>(readFrames.value(DataFrame::Metadata));

            if (previousFrame + 1 != colorFrame->getIndex())
                qDebug() << "Frame Skip" << colorFrame->getIndex();

            qDebug() << "Processing frame" << colorFrame->getIndex();

            // Process
            QList<int> users = skeletonFrame->getAllUsersId();

            // Work with the user inside of the Bounding Box
            if (!users.isEmpty() && !metadataFrame->boundingBoxes().isEmpty() /*&& colorFrame->getIndex() > 140*/)
            {
                int firstUser = users.at(0);
                shared_ptr<Skeleton> skeleton = skeletonFrame->getSkeleton(firstUser);

                // Get ROIs
                BoundingBox bb = metadataFrame->boundingBoxes().first();
                shared_ptr<ColorFrame> roiColor = colorFrame->subFrame(bb);
                shared_ptr<DepthFrame> roiDepth = depthFrame->subFrame(bb);
                shared_ptr<MaskFrame> roiMask = maskFrame->subFrame(bb);

                QString fileName = "U" + QString::number(instance_info->getActor()) +
                                  "_C" + QString::number(instance_info->getCamera()) +
                                  "_F" + QString::number(roiColor->getIndex());

                // Show
                cv::Mat color_mat(roiColor->height(), roiColor->width(), CV_8UC3,
                                  (void*) roiColor->getDataPtr(), roiColor->getStride());

                cv::imshow("img1", color_mat);
                cv::waitKey(1);
                QCoreApplication::processEvents();

                // Save color as JPEG
                QImage image( (uchar*) roiColor->getDataPtr(), roiColor->width(), roiColor->height(),
                             roiColor->getStride(), QImage::Format_RGB888);
                image.save("data/" + fileName + "_color.jpg");

                // Save depth
                /*cv::Mat depth_mat(roiDepth->height(), roiDepth->width(), CV_16UC1,
                                  (uchar*) roiDepth->getDataPtr(), roiDepth->getStride());
                cv::imwrite(QString("data/" + fileName + "_depth.png").toStdString(), depth_mat);*/
                QFile depthFile("data/" + fileName + "_depth.bin");
                QByteArray depth_data = roiDepth->toBinary();
                depthFile.open(QIODevice::WriteOnly);
                depthFile.write(depth_data);
                depthFile.close();

                // Save mask
                QFile maskFile("data/" + fileName + "_mask.bin");
                QByteArray mask_data = roiMask->toBinary();
                maskFile.open(QIODevice::WriteOnly);
                maskFile.write(mask_data);
                maskFile.close();

                // Save Skeleton
                QFile file("data/" + fileName + "_skel.bin");
                QByteArray skel_data = skeleton->toBinary();
                file.open(QIODevice::WriteOnly);
                file.write(skel_data);
                file.close();
            }

            previousFrame = colorFrame->getIndex();
        }

        // Close Instances
        for (shared_ptr<StreamInstance> instance : instances) {
            instance->close();
        }
    }

    qDebug() << "Parse Finished!";
}



int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    CoreLib_InitResources();
    //parseMSRAction3D(argv[1]);
    //parseCAVIAR4REID(argv[1]);
    //parseDAI4REID(argv[1]);
    //parseMSRAction3D_Quaternions(argv[1]);
    parseIAS_LAB_RGBD_ID((argv[1]));
}

