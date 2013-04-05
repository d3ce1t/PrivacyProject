#include "MSRDailyActivity3DInstance.h"
#include "DepthFrame.h"
#include <cstddef>
#include <math.h>
#include <stdint.h>
#include <QDebug>

using namespace std;

namespace dai {

MSRDailyActivity3DInstance::MSRDailyActivity3DInstance(const InstanceInfo &info)
    : DataInstance(info)
{
    m_nColumns = 0;
    m_nRows = 0;
    m_nFrames = 0;
    m_frameIndex = 0;
}

MSRDailyActivity3DInstance::~MSRDailyActivity3DInstance()
{
    close();
}

void MSRDailyActivity3DInstance::open()
{
    QString instancePath = m_info.getDatasetPath() + "/" + m_info.getFileName();
    m_file.open(instancePath.toStdString().c_str(), ios::in|ios::binary);

    if (!m_file.is_open()) {
        cerr << "Error opening file" << endl;
        return;
    }

    m_file.read((char *) &m_nFrames, 4);
    m_file.read((char *) &m_nColumns, 4);
    m_file.read((char *) &m_nRows, 4);
}

void MSRDailyActivity3DInstance::close()
{
    if (m_file.is_open()) {
        m_file.close();
        m_nFrames = 0;
        m_nColumns = 0;
        m_nRows = 0;
        m_frameIndex = 0;
    }
}

int MSRDailyActivity3DInstance::getTotalFrames()
{
    return m_nFrames;
}

bool MSRDailyActivity3DInstance::hasNext()
{
    if (m_frameIndex < m_nFrames)
        return true;

    return false;
}

DepthFrame *MSRDailyActivity3DInstance::nextFrame()
{
    DepthFrame* result = 0;

    if (m_frameIndex < m_nFrames)
    {
        result = new DepthFrame(m_nColumns, m_nRows);
        result->setIndex(m_frameIndex);

        // Read Data from File
        int* tempRow = new int[m_nColumns];
        uint8_t* tempRowID = new uint8_t[m_nColumns];

        for(int r=0; r<m_nRows; r++) {

            m_file.read((char *) tempRow, 4*m_nColumns);
            //fread(tempRow, 4, nColumns, fileDescriptor);
            m_file.read((char*) tempRowID, 1*m_nColumns);
            //fread(tempRowID, 1, nColumns, fileDescriptor);

            for(int c=0; c<m_nColumns; c++) {
                result->setItem(r, c, tempRow[c]);
            }
        }

        delete[] tempRow;
        tempRow = NULL;

        delete[] tempRowID;
        tempRowID = NULL;

        m_frameIndex++;
    }
    else {
        close();
    }

    return result;
}

} // End Namespace
