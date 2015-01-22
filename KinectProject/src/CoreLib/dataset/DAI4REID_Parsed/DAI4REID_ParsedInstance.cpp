#include "DAI4REID_ParsedInstance.h"
#include "dataset/DatasetMetadata.h"
#include "types/ColorFrame.h"
#include "types/DepthFrame.h"
#include "types/MaskFrame.h"
#include "types/SkeletonFrame.h"
#include <opencv2/opencv.hpp>
#include <QFile>
#include "Utils.h"

namespace dai {

DAI4REID_ParsedInstance::DAI4REID_ParsedInstance(const InstanceInfo &info)
    : DataInstance(info, DataFrame::Color, -1, -1)
{
    m_width = 0;
    m_height = 0;
    m_open = false;
}

DAI4REID_ParsedInstance::~DAI4REID_ParsedInstance()
{
    m_width = 0;
    m_height = 0;
    closeInstance();
}

bool DAI4REID_ParsedInstance::is_open() const
{
    return m_open;
}

bool DAI4REID_ParsedInstance::hasNext() const
{
    return true;
}

bool DAI4REID_ParsedInstance::openInstance()
{
    bool result = false;
    QString instancePath = m_info.parent().getPath() + "/";

    if (QFile::exists(instancePath + m_info.getFileName(DataFrame::Color)) &&
            QFile::exists(instancePath + m_info.getFileName(DataFrame::Depth)) &&
                QFile::exists(instancePath + m_info.getFileName(DataFrame::Mask)) &&
                    QFile::exists(instancePath + m_info.getFileName(DataFrame::Skeleton))) {
        result = true;
        m_open = true;
    }

    return result;
}

void DAI4REID_ParsedInstance::closeInstance()
{
     m_open = false;
}

void DAI4REID_ParsedInstance::restartInstance()
{
}

void DAI4REID_ParsedInstance::nextFrame(QHashDataFrames &output)
{
    // Read Color File
    QString instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Color);
    cv::Mat color_mat = cv::imread(instancePath.toStdString());
    cv::cvtColor(color_mat, color_mat, CV_BGR2RGB);
    ColorFrame srcColor(color_mat.cols, color_mat.rows, (RGBColor*) color_mat.data);
    shared_ptr<ColorFrame> dstColor = make_shared<ColorFrame>();
    *dstColor = srcColor; // Copy
    output.insert(DataFrame::Color, dstColor);

    // Read Depth File
    instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Depth);
    QFile depthFile(instancePath);
    depthFile.open(QIODevice::ReadOnly);
    QByteArray buffer = depthFile.readAll();
    depthFile.close();
    shared_ptr<DepthFrame> depthFrame = make_shared<DepthFrame>();
    depthFrame->loadData(buffer);
    depthFrame->setDistanceUnits(dai::DISTANCE_MILIMETERS);
    output.insert(DataFrame::Depth, depthFrame);
    /*instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Depth);
    cv::Mat depth_mat = cv::imread(instancePath.toStdString()); //, CV_LOAD_IMAGE_GRAYSCALE);
    qDebug() << depth_mat.type() << depth_mat.depth() << depth_mat.channels();
    DepthFrame srcDepth(depth_mat.cols, depth_mat.rows, (uint16_t*) depth_mat.data, depth_mat.step);
    shared_ptr<DepthFrame> depthFrame = make_shared<DepthFrame>();
    *depthFrame = srcDepth; // Copy
    depthFrame->setDistanceUnits(dai::MILIMETERS);
    output.insert(DataFrame::Depth, depthFrame);*/

    // Read Mask File
    instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Mask);
    QFile maskFile(instancePath);
    maskFile.open(QIODevice::ReadOnly);
    buffer = maskFile.readAll();
    maskFile.close();
    shared_ptr<MaskFrame> maskFrame = make_shared<MaskFrame>();
    maskFrame->loadData(buffer);
    output.insert(DataFrame::Mask, maskFrame);

    // Read Skeleton File
    instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Skeleton);
    QFile skeletonFile(instancePath);
    skeletonFile.open(QIODevice::ReadOnly);
    buffer = skeletonFile.readAll();
    skeletonFile.close();
    shared_ptr<Skeleton> skeleton = Skeleton::fromBinary(buffer);
    shared_ptr<SkeletonFrame> skeletonFrame = make_shared<SkeletonFrame>();
    skeletonFrame->setSkeleton(1, skeleton);
    output.insert(DataFrame::Skeleton, skeletonFrame);
}

} // End namespace
