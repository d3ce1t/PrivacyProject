#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString currentDataSetDir = argv[1];

    QDir currentDir(currentDataSetDir);
    currentDir.setFilter(QDir::Files);

    QStringList entries = currentDir.entryList();

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

    return a.exec();
}
