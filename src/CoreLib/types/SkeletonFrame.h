#ifndef SKELETONFRAME_H
#define SKELETONFRAME_H

#include "types/DataFrame.h"
#include "types/Skeleton.h"
#include <QMap>
#include <memory>
//#include <QFile>

using namespace std;

namespace dai {

class SkeletonFrame;
typedef shared_ptr<SkeletonFrame> SkeletonFramePtr;

class SkeletonFrame : public DataFrame
{
    QMap<int, SkeletonPtr> m_hashSkeletons;
    int m_width;
    int m_height;

public:

    static SkeletonFramePtr fromBinary(const QByteArray &buffer);

    SkeletonFrame();
    SkeletonFrame(int width, int height);
    SkeletonFrame(const SkeletonFrame& other);
    int width() const {return m_width;}
    int height() const {return m_height;}
    shared_ptr<DataFrame> clone() const override;
    SkeletonPtr getSkeleton(int userId) const;
    QList<SkeletonPtr> skeletons() const;
    void setSkeleton(int userId, const dai::SkeletonPtr skeleton);
    QList<int> getAllUsersId() const;
    void clear();
    QByteArray toBinary() const;

    // Overload operators
    SkeletonFrame& operator=(const SkeletonFrame& other);
};

} // End Namespace

#endif // SKELETONFRAME_H
