#ifndef TESTS_H
#define TESTS_H

#include "types/DataFrame.h"

namespace dai {

class Tests
{
public:
    void test1();
    void test2();
    void test3();
    void show_different_skel_resolutions();
    void approach1(QHashDataFrames& frames);
    void approach2(QHashDataFrames& frames);
    void approach3(QHashDataFrames& frames);
    void approach4(QHashDataFrames& frames);
    void approach5(QHashDataFrames& frames);
    void approach6(QHashDataFrames& frames);
};

} // End Namespace

#endif // TESTS_H
