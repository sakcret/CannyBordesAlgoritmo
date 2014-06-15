#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mPicture = 0;
    mCurrentPath = "";

    //ui->scrollArea->setWidgetResizable(true);
    //ui->lblImage->setScaledContents(true);
    /*ui->lblImage->setAlignment(Qt::AlignCenter);
    ui->lblImage->setSizePolicy(QSizePolicy::Expanding,
                                  QSizePolicy::Expanding);*/
    //ui->scrollArea->setMinimumSize(240, 160);

    // Commands
    connect(ui->cmdLoad, SIGNAL(triggered()), SLOT(slotLoad()));
    connect(ui->cmdSave, SIGNAL(triggered()), SLOT(slotSave()));
    connect(ui->cmdCanny, SIGNAL(clicked()), SLOT(slotCanny()));
    connect(ui->cmdShowOriginal, SIGNAL(toggled(bool)), SLOT(slotUpdate()));
    connect(ui->cmdReload, SIGNAL(clicked()), SLOT(slotReload()));
    connect(ui->cmdReloadAction, SIGNAL(triggered()), SLOT(slotReload()));
    //connect(ui->cmdAbout, SIGNAL(triggered()), SLOT(slotAbout()));

    // Hysteresis
    connect(ui->chkHysteresis, SIGNAL(clicked()), SLOT(slotUpdate()));
    connect(ui->sliderHysteresisLow, SIGNAL(sliderMoved(int)), SLOT(slotUpdate()));
    connect(ui->sliderHysteresisHigh, SIGNAL(sliderMoved(int)), SLOT(slotUpdate()));
    connect(ui->spinHysteresisLow, SIGNAL(editingFinished()), SLOT(slotUpdateFromSpins()));
    connect(ui->spinHysteresisHigh, SIGNAL(editingFinished()), SLOT(slotUpdateFromSpins()));
    connect(ui->cmdDefault,SIGNAL(clicked()),this, SLOT(slotCannyDefault()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent * e)
{
    redisplay();
    QMainWindow::resizeEvent(e);
}

void MainWindow::slotLoad(QString path)
{
    if(mPicture != 0)
        delete mPicture;

    if(path == "")
        mCurrentPath = QFileDialog::getOpenFileName(this, "Please select an image...", ".", "Images (*.png *.bmp *.gif *.xpm *.jpg)");
    else
        mCurrentPath = path;

    if(mCurrentPath == "") return;

    mPicture = new CImage(mCurrentPath);
    redisplay();
}

void MainWindow::slotReload()
{
    slotLoad(mCurrentPath);
}

void MainWindow::slotSave()
{
    bool fail = (mPicture == 0);
    if(!fail)
        if(mPicture->mImage->isNull())
            fail = true;
    if(fail) {
        QMessageBox::critical(this, "Error", "No file to save!");
        return;
    }

    QString path = QFileDialog::getSaveFileName(this, "Please select a path to save to...", "out.jpg", "*.jpg");
    mPicture->mImage->save(path);
}

void MainWindow::slotCanny()
{
    if(mPicture == 0) return;

    mPicture->canny(
                ui->spinBlurSigma->value(),
                ui->chkR->isChecked(),
                ui->chkG->isChecked(),
                ui->chkB->isChecked());

    if(ui->cmdShowOriginal->isChecked())
        ui->cmdShowOriginal->setChecked(false);
    else
        slotUpdate();
}

void MainWindow::slotCannyDefault()
{
    if(mPicture == 0) return;

    mPicture->canny(
                //blur sigma
                1.000,
                //R is checked
                true,
                //G is checked
                true,
                //B is checked
                true);
    double low = 0.007;
    double high = 0.099;

    mPicture->useHysteresis(low, high);

    if(mPicture == 0) return;

    if(ui->cmdShowOriginal->isChecked())
        mDisplayImage = QPixmap::fromImage(*mPicture->mOriginalImage);
    else
        mDisplayImage = QPixmap::fromImage(*mPicture->mImage);
    ui->lblImage->setPixmap(
                mDisplayImage.scaled(ui->lblImage->size(),
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation));
}

void MainWindow::redisplay()
{
    if(mPicture == 0) return;

    if(ui->cmdShowOriginal->isChecked())
        mDisplayImage = QPixmap::fromImage(*mPicture->mOriginalImage);
    else
        mDisplayImage = QPixmap::fromImage(*mPicture->mImage);
    ui->lblImage->setPixmap(
                mDisplayImage.scaled(ui->lblImage->size(),
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation));
}

void MainWindow::slotUpdate()
{
    double low = ui->sliderHysteresisLow->value()/1000.;
    double high = ui->sliderHysteresisHigh->value()/1000.;
    if(low > high) {
        low = high;
        ui->sliderHysteresisLow->setValue(ui->sliderHysteresisHigh->value());
    }

    ui->spinHysteresisHigh->setValue(high);
    ui->spinHysteresisLow->setValue(low);

    if(mPicture == 0) return;

    if(!ui->cmdShowOriginal->isChecked()) {
        if(ui->chkHysteresis->isChecked()) {
            mPicture->useHysteresis(low, high);
            qDebug()<<"Hysteresis "<<low<<" "<<high;
        } else{
            mPicture->useSuppressed();
            qDebug("supress");}
    }

    redisplay();
}

void MainWindow::slotUpdateFromSpins()
{
    ui->sliderHysteresisLow->setValue(ui->spinHysteresisLow->value()*1000);
    ui->sliderHysteresisHigh->setValue(ui->spinHysteresisHigh->value()*1000);
}
