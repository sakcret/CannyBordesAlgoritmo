#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "globals.h"
#include "CImage.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    CImage * mPicture;
    QPixmap mDisplayImage;
    QString mCurrentPath;

    void redisplay();

    virtual void resizeEvent(QResizeEvent * e);

protected slots:
    void slotLoad(QString path = "");
    void slotSave();
    void slotCanny();
    void slotReload();
    void slotUpdate();
    void slotUpdateFromSpins();
    void slotCannyDefault();

private:
    Ui::MainWindow *ui;


};

#endif // MAINWINDOW_H
