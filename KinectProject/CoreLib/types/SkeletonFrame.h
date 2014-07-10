#ifndef SKELETONFRAME_H
#define SKELETONFRAME_H

#include "types/DataFrame.h"
#include "types/Skeleton.h"
#include <QHash>
#include <memory>
#include <QFile>

using namespace std;

namespace dai {

class SkeletonFrame : public DataFrame
{
public:
    // Constructor
    SkeletonFrame();
    SkeletonFrame(const SkeletonFrame& other);

    // Methods
    shared_ptr<DataFrame> clone() const override;
    shared_ptr<dai::Skeleton> getSkeleton(int userId);
    void setSkeleton(int userId, const shared_ptr<dai::Skeleton> skeleton);
    QList<int> getAllUsersId() const;
    void clear();
    void write(QFile &of) const;

    // Overload operators
    SkeletonFrame& operator=(const SkeletonFrame& other);

private:
    QHash<int, shared_ptr<dai::Skeleton>> m_hashSkeletons;
};

} // End Namespace

#endif // SKELETONFRAME_H
