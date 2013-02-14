#ifndef DSSAMPLE_H
#define DSSAMPLE_H

namespace DataSet {

    class Sample
    {
    public:
        enum Type { RGB, Depth, Skeleton, NO_TYPE};

        Sample();
        Sample(int ncols, int nrows, int frameIndex, Type type);
        Sample(const Sample& other);
        ~Sample(void);

        Sample& operator=(const Sample &other);

        int GetNRows() const {return nRows;}
        int GetNCols() const {return nColumns;}
        int getFrameIndex() const {return frameIndex;}
        Type getType() const {return type;}

        void SetItem(int r, int c, int val);
        int GetItem(int r, int c) const;
        int NumberOfNonZeroPoints() const;
        float AvgNonZeroDepth() const;
        int MaxValue() const;
        int MinValue() const;


    protected:
        Sample::Type type;
        int nColumns;
        int nRows;
        int frameIndex;
        int* rawData;
    };
}
#endif
