#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <iostream>
#include "dataset/InstanceInfo.h"
#include "dataset/DatasetMetadata.h"
#include <QMap>

using namespace std;

dai::InstanceInfo* findInstance(int activity, int actor, int sample, QMap<int, QMap<int, QMap<int, dai::InstanceInfo *> > > list);

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

void parseMSRAction3D(const QStringList& entries)
{
    for( QStringList::ConstIterator entry=entries.begin(); entry!=entries.end(); ++entry )
    {
        // a01_s01_e01_sdepth.bin
        QString fileEntry = *entry;
        QString activity = fileEntry.mid(1, 2);
        QString subject = fileEntry.mid(5, 2);
        QString extension = fileEntry.mid(fileEntry.indexOf(".")+1);
        QString sample = fileEntry.mid(fileEntry.indexOf("_e")+2, 2);

        QString type = "unknown";

        if (extension == "bin") {
            type = "depth";
        } else if (extension == "txt") {
            type = "skeleton";
        }

        if (type == "unknown")
            continue;

        cout << "<instance " <<
                "type=\"" << type.toStdString() << "\" " <<
                "activity=\"" << activity.toStdString() << "\" " <<
                "actor=\"" << subject.toStdString() << "\" " <<
                "sample=\"" << sample.toStdString()<< "\">" << endl;
        cout << "\t<file>" << fileEntry.toStdString() << "</file>" << endl;
        cout << "</instance>" << endl;
    }
}

void parseHuDaAct(const QStringList& entries)
{
    QHash<QString, int> map = {
        { "L",   1}, // L: enter room
        { "E",   2}, // E: exit the room
        { "P",   3}, // P: make a phone call
        { "B",   4}, // B: go to bed
        { "G",   5}, // G: get up from bed
        { "I",   6}, // I: sit down
        { "T",   7}, // T: stand up
        { "O",   8}, // O: mop floor
        { "M",   9}, // M: eat meal
        { "D",  10}, // D: put on jacket
        { "N",  11}, // N: take off jacket
        { "K",  12}, // K: drink water
        { "BG", 13}, // BG: background activity
        {"SBG", 14}, // SBG: background activity
        { "CB", 15}, // CB: background activity
        {"SCB", 16}  // SCB: background activity
    };

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
        dai::InstanceInfo* instanceInfo = findInstance(map[activity], subject, sample, list);

        if (!instanceInfo) {
            instanceInfo = new dai::InstanceInfo;
            instanceInfo->setActivity(map[activity]);
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
                        "activity=\"" << info->getActivity() << "\" " <<
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

dai::InstanceInfo* findInstance(int activity, int actor, int sample, QMap<int, QMap<int, QMap<int, dai::InstanceInfo*> > > list)
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

                if (instance->getActivity() == activity && instance->getActor() == actor && instance->getSample() == sample) {
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
    QCoreApplication a(argc, argv);
    QString currentDataSetDir = argv[1];

    QDir currentDir(currentDataSetDir);
    currentDir.setFilter(QDir::Files);

    QStringList entries = currentDir.entryList();
    parseHuDaAct(entries);

    return a.exec();
}

