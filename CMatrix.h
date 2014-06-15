#ifndef CMATRIX_H
#define CMATRIX_H

#include "globals.h"

template<typename T> class CArray          // e.g. matrix row
{
public:
    uint mSize;
    T * mItems;

    CArray(uint size);
    ~CArray();

    T& operator[](uint index) { return mItems[index]; }

    void operator/=(T quotient);

    T sum();
};

template<typename T> class CMatrix
{
public:
    uint mHeight, mWidth;
    CArray<T> ** mRows;

    CMatrix(CMatrix<T> * copyFrom);
    CMatrix(uint height, uint width);
    CMatrix(uint height, uint width, T initialValue);
    CMatrix(QImage * image, bool useR = true, bool useG = true, bool useB = true);
    ~CMatrix();

    CArray<T> & operator[](uint index) { return *mRows[index]; } 
    T& at(uint x, uint y) { return (*mRows[x]).operator [](y); }     // Row, column
    void set(uint x, uint y, T value) { return mRows[x][y]; }        // Row, column

    void operator/=(T quotient);
    void operator+=(CMatrix<T>& summand);
    void squareElementsInPlace();
    void squareRootElementsInPlace();

    static CMatrix<T> * atan2(CMatrix<T>& y, CMatrix<T>& x);

    T sum();

    CMatrix<T> * filterBy(CMatrix<T>& kernel);

    QImage * toNewImage(bool rescale = true);  // Be sure to delete
    void debugPrint();

};

// Useful typedef:
typedef CMatrix<double> CMatD;

template<typename T> CArray<T>::CArray(uint size)
        : mSize(size)
{
    mItems = new T[mSize];
}

template<typename T> CArray<T>::~CArray()
{
    delete [] mItems;
}

template<typename T> void CArray<T>::operator/=(T quotient)
{
    for(uint i = 0; i < mSize; i++)
        mItems[i] /= quotient;
}

template<typename T> T CArray<T>::sum()
{
    T r = 0;     // T must have a zero
    for(uint i = 0; i < mSize; i++)
        r += mItems[i];
    return r;
}

template<typename T> CMatrix<T>::CMatrix(CMatrix<T> * copyFrom)
{
    mHeight = copyFrom->mHeight;
    mWidth = copyFrom->mWidth;
    mRows = new CArray<T>*[mHeight];
    for(uint i = 0; i < mHeight; i++) {
        mRows[i] = new CArray<T>(mWidth);
        for(uint j = 0; j < mWidth; j++) {
            at(i,j) = copyFrom->at(i,j);
        }
    }
}

template<typename T> CMatrix<T>::CMatrix(uint height, uint width)
        : mHeight(height), mWidth(width)
{
    mRows = new CArray<T>*[mHeight];
    for(uint i = 0; i < mHeight; i++)
        mRows[i] = new CArray<T>(mWidth);
}

template<typename T> CMatrix<T>::CMatrix(uint height, uint width, T initialValue)
        : mHeight(height), mWidth(width)
{
    mRows = new CArray<T>*[mHeight];
    for(uint i = 0; i < mHeight; i++) {
        mRows[i] = new CArray<T>(mWidth);
        for(uint j = 0; j < mWidth; j++) {
            at(i,j) = initialValue;
        }
    }
}

template<typename T> CMatrix<T>::CMatrix(QImage * im, bool useR, bool useG, bool useB)
{
    mHeight = im->height();
    mWidth = im->width();

    if(!(useR || useG || useB))
        useR = useG = useB;

    mRows = new CArray<T>*[mHeight];
    for(uint i = 0; i < mHeight; i++) {
        mRows[i] = new CArray<T>(mWidth);
        for(uint j = 0; j < mWidth; j++) {
            uint p = im->pixel(j,i);
            at(i,j) = 0;
            double totalW = 0;
            if(useR) { at(i,j) += (p&0xFF)*0.11; totalW += 0.11; }
            if(useG) { at(i,j) += ((p&0xFF00)>>8)*0.59; totalW += 0.59; }
            if(useB) { at(i,j) += ((p&0xFF0000)>>16)*0.3; totalW += 0.3; }
            at(i,j) /= totalW * 256.;
        }
    }
}

template<typename T> CMatrix<T>::~CMatrix()
{
    for(uint i = 0; i < mHeight; i++)
        delete mRows[i];
    delete [] mRows;
}

template<typename T> void CMatrix<T>::operator/=(T quotient)
{
    for(uint i = 0; i < mHeight; i++)
        (*mRows[i]) /= quotient;
}

template<typename T> void CMatrix<T>::operator+=(CMatrix<T>& summand)
{
    for(uint i = 0; i < mHeight; i++)
        for(uint j = 0; j < mWidth; j++)
            at(i, j) += summand.at(i, j);
}

template<typename T> void CMatrix<T>::squareElementsInPlace()
{
    for(uint i = 0; i < mHeight; i++)
        for(uint j = 0; j < mWidth; j++)
            at(i, j) *= at(i, j);
}

template<typename T> void CMatrix<T>::squareRootElementsInPlace()
{
    for(uint i = 0; i < mHeight; i++)
        for(uint j = 0; j < mWidth; j++)
            at(i, j) = sqrt(at(i, j));
}

template<typename T> CMatrix<T> * CMatrix<T>::atan2(CMatrix<T>& y, CMatrix<T>& x)
{
    CMatrix<T> * out = new CMatrix<T>(y.mHeight, y.mWidth);
    for(uint i = 0; i < y.mHeight; i++)
        for(uint j = 0; j < y.mWidth; j++)
            out->at(i, j) = ::atan2(y[i][j], x[i][j]);
    return out;
}

template<typename T> T CMatrix<T>::sum()
{
    T r = 0;    // T must have a zero
    for(uint i = 0; i < mHeight; i++)
        r += mRows[i]->sum();
    return r;
}

// Implicitly 0-padded
template<typename T> CMatrix<T>* CMatrix<T>::filterBy(CMatrix<T>& kernel)
{
    CMatrix<T> * out = new CMatrix<T>(mHeight, mWidth);

    if((kernel.mWidth & 1 == 0) | (kernel.mHeight & 1 == 0)) {
        QMessageBox::critical(0, "Error", "Kernels must have odd size");
        return out;
    }

    int rangeX = (kernel.mHeight-1)/2, rangeY = (kernel.mWidth-1)/2;

    for(uint i = 0; i < mHeight; i++) {
        for(uint j = 0; j < mWidth; j++) {
            out->at(i, j) = 0;
            for(int x = -rangeX; x <= rangeX; x++) {
                int posX = x + i;
                if(posX < 0 || posX >= (int)mHeight) continue;
                for(int y = -rangeY; y <= rangeY; y++) {
                    int posY = y + j;
                    if(posY < 0 || posY >= (int)mWidth) continue;
                    out->at(i, j) += at(posX, posY)*kernel[rangeX+x][rangeY+y];
                }
            }
        }
    }

    return out;
}

template<typename T> QImage * CMatrix<T>::toNewImage(bool rescale)
{
    QImage * out = new QImage(mWidth, mHeight, QImage::Format_RGB32);

    double baseline = 0, scaleFactor = 255.;

    if(rescale) {
        T min = at(0,0), max = at(0,0);
        for(uint i = 0; i < mHeight; i++) {
            for(uint j = 0; j < mWidth; j++) {
                if(min > at(i,j)) min = at(i,j);
                if(max < at(i,j)) max = at(i,j);
            }
        }
        baseline = min;
        if(max != min)
            scaleFactor = 255./(max - min);
        else
            scaleFactor = 255.;
    }
    for(uint i = 0; i < mHeight; i++) {
        for(uint j = 0; j < mWidth; j++) {
            uint c = (uint)ceil((at(i,j)-baseline)*scaleFactor);
            out->setPixel(j, i, qRgb(c,c,c));
        }
    }
    return out;
}

template<typename T> void CMatrix<T>::debugPrint()
{
    for(uint i = 0; i < mHeight; i++) {
        for(uint j = 0; j < mWidth; j++) {
            qDebug() << "(" << i << "," << j << ")" << at(i,j);
        }
    }
}

#endif // CMATRIX_H
