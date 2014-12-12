#ifndef SKELETONFRAME_H
#define SKELETONFRAME_H

#include "types/DataFrame.h"
#include "types/Skeleton.h"
#include <QHash>
#include <memory>
//#include <QFile>

using namespace std;

namespace dai {

class SkeletonFrame : public DataFrame
{
    QHash<int, dai::SkeletonPtr> m_hashSkeletons;

public:
    // Constructor
    SkeletonFrame();
    SkeletonFrame(const SkeletonFrame& other);

    // Methods
    shared_ptr<DataFrame> clone() const override;
    dai::SkeletonPtr getSkeleton(int userId);
    void setSkeleton(int userId, const dai::SkeletonPtr skeleton);
    QList<int> getAllUsersId() const;
    void clear();

    // Overload operators
    SkeletonFrame& operator=(const SkeletonFrame& other);
};

typedef std::shared_ptr<dai::SkeletonFrame> SkeletonFramePtr;

} // End Namespace

#endif // SKELETONFRAME_H
