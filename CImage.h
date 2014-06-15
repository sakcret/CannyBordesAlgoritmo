#ifndef CIMAGE_H
#define CIMAGE_H

#include "globals.h"

class CImage
{
public:
    uint mWidth, mHeight;
    QImage * mOriginalImage, * mImage;
    CMatD * mSuppressed;

    CImage(uint w, uint h);
    CImage(QString file);
    ~CImage();

    void canny(double blurSigma, bool useR, bool useG, bool useB);

    void useSuppressed();
    void useHysteresis(double thresholdLow, double thresholdHigh);

    CMatD * suppression(CMatD& grad, CMatrix<int>& thetaClamped);
    CMatrix<int> * hysteresis(CMatD& grad, double thresholdLow, double thresholdHigh);

    CMatD gaussianFilter(double sigma);

    void timing(QString info, bool restart = false);
};

#endif // CIMAGE_H
