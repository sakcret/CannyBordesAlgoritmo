#include "CImage.h"

CImage::CImage(uint w, uint h)
        : mWidth(w), mHeight(h)
{
    mSuppressed = 0;
    mImage = new QImage(w, h, QImage::Format_RGB32);
    mOriginalImage = new QImage(w, h, QImage::Format_RGB32);
}

CImage::CImage(QString file)
{
    mSuppressed = 0;
    mImage = new QImage(file);
    if(mImage->isNull())
        QMessageBox::critical(0, "Oops", "Couldn't load that, sorry!");
    mOriginalImage = new QImage(file);
}

CImage::~CImage()
{
    delete mImage;
    if(mSuppressed != 0) delete mSuppressed;
}

void CImage::useSuppressed()
{
    if(mSuppressed == 0) return;
    delete mImage;
    mImage = mSuppressed->toNewImage();
}

void CImage::useHysteresis(double thresholdLow, double thresholdHigh)
{
    if(mSuppressed == 0) return;
    CMatrix<int> * traced = hysteresis(*mSuppressed, thresholdLow, thresholdHigh);

    delete mImage;
    mImage = traced->toNewImage();

    delete traced;
}

void CImage::canny(double blurSigma, bool useR, bool useG, bool useB)
{
    timing("Starting Canny edge detection.", true);
    CMatD gaussian = gaussianFilter(blurSigma);

    CMatD image(mImage, useR, useG, useB);
    mHeight = image.mHeight;
    mWidth = image.mWidth;

    timing("Matrix constructed.");

    CMatD * filtered = image.filterBy(gaussian);

    timing("Matrix filtered.");

    CMatD prewittX(3, 3, 0), prewittY(3, 3, 0);
    for(uint i = 0; i < 3; i++) {
        prewittX[i][0] = -1;
        prewittX[i][2] = +1;
        prewittY[0][i] = -1;
        prewittY[2][i] = +1;
    }

    CMatD * gradX = filtered->filterBy(prewittX);
    CMatD * gradY = filtered->filterBy(prewittY);

    CMatD * grad = new CMatD(gradX);
    CMatD * temp = new CMatD(gradY);

    grad->squareElementsInPlace();
    temp->squareElementsInPlace();
    grad->operator +=(*temp);
    grad->squareRootElementsInPlace();

    CMatD * theta = CMatD::atan2(*gradY, *gradX);
    CMatrix<int> * thetaClamped = new CMatrix<int>(mHeight, mWidth);

    timing("Gradient and angle calculated.");

    for(uint i = 0; i < mHeight; i++)
        for(uint j = 0; j < mWidth; j++) {
            double t = theta->at(i,j);
            if(t < 0) t += M_PI;

            if(t <= M_PI/8) thetaClamped->at(i,j) = 3;
            else if(t <= 3*M_PI/8) thetaClamped->at(i,j) = 2;
            else if(t <= 5*M_PI/8) thetaClamped->at(i,j) = 1;
            else if(t <= 7*M_PI/8) thetaClamped->at(i,j) = 4;
            else thetaClamped->at(i,j) = 3;
        }

    if(mSuppressed != 0) delete mSuppressed;
    mSuppressed = suppression(*grad, *thetaClamped);

    timing("Clamped and suppressed.");

    delete mImage;
    mImage = image.toNewImage();

    delete theta;
    delete grad;
    delete temp;
    delete gradX;
    delete gradY;
    delete filtered;
}

CMatD * CImage::suppression(CMatD& grad, CMatrix<int>& theta)
{
    CMatD * out = new CMatD(&grad);
    for(uint i = 0; i < mHeight; i++)
        for(uint j = 0; j < mWidth; j++) {
            int ax, ay, bx, by;
            int angle = theta[i][j];
            if(angle == 1) {
                ax = i+1; ay = j;
                bx = i-1; by = j;
            } else if(angle == 2) {
                ax = i+1; ay = j+1;
                bx = i-1; by = j-1;
            } else if(angle == 3) {
                ax = i; ay = j+1;
                bx = i; by = j-1;
            } else if(angle == 4) {
                ax = i-1; ay = j+1;
                bx = i+1; by = j-1;
            } else { qDebug() << angle; QMessageBox::critical(0, "Error", "Corrupt angle."); return out; }

            if(ax < 0 || ax >= (int)mHeight || ay < 0 || ay >= (int)mWidth) continue;
            else if(grad[ax][ay] > grad[i][j]) { out->at(i,j) = 0; continue; }

            if(bx < 0 || bx >= (int)mHeight || by < 0 || by >= (int)mWidth) continue;
            else if(grad[bx][by] > grad[i][j]) { out->at(i,j) = 0; continue; }
        }
    return out;
}

CMatrix<int> * CImage::hysteresis(CMatD& grad, double thresholdLow, double thresholdHigh)
{
    queue< pair<int, int> > nodes;
    CMatrix<int> * out = new CMatrix<int>(mHeight, mWidth, 0);
    for(uint i = 0; i < mHeight; i++)
        for(uint j = 0; j < mWidth; j++) {
            if((grad[i][j] >= thresholdHigh) && out->at(i,j) != 1) {
                nodes.push(pair<uint, uint>(i,j));
                while(!nodes.empty()) {
                    pair<int, int> node = nodes.front();
                    nodes.pop();
                    int x = node.first, y = node.second;
                    if(x < 0 || x >= (int)mHeight || y < 0 || y >= (int)mWidth) continue;
                    if(grad[x][y] < thresholdLow) continue;
                    if(out->at(x,y) != 1) {
                        out->at(x,y) = 1;
                        nodes.push(pair<uint, uint>(x+1,y-1));
                        nodes.push(pair<uint, uint>(x+1,y  ));
                        nodes.push(pair<uint, uint>(x+1,y+1));
                        nodes.push(pair<uint, uint>(x  ,y+1));
                        nodes.push(pair<uint, uint>(x  ,y-1));
                        nodes.push(pair<uint, uint>(x-1,y-1));
                        nodes.push(pair<uint, uint>(x-1,y  ));
                        nodes.push(pair<uint, uint>(x-1,y+1));
                    }
                }
            }
        }
    return out;
}

CMatD CImage::gaussianFilter(double sigma)
{
    /* The Gaussian filter is separable, but for simplicity we implement it as a square filter */
    int n = (int)(2 * floor( (float)sqrt(-log(0.1) * 2 * (sigma*sigma)) ) + 1);
    CMatD r(n,n);

    for(int i = -(n-1)/2; i <= (n-1)/2; i++)
        for(int j = -(n-1)/2; j <= (n-1)/2; j++) {
            r[(n-1)/2+i][(n-1)/2+j] = (float)exp(-((float)((i*i)+(j*j))/(2*(sigma*sigma))));
        }

    r /= r.sum();

    return r;
}

void CImage::timing(QString info, bool restart)
{
    static QTime timerInitial, timer;
    if(restart) {
        timer.start();
        timerInitial.start();
        qDebug() << endl << info << " @ " << timerInitial.toString("hh:mm:ss");
        return;
    }
    qDebug() << info << " @ " << QTime::currentTime().toString("hh:mm:ss") << " (" << timer.elapsed()/1000. << " seconds)";
    timer.start();
}
