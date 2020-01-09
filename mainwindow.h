#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QCamera>
#include <QMainWindow>
#include <QCameraImageCapture>
#include <QMediaRecorder>
#include <QScopedPointer>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <opencv2/opencv.hpp>
#include <QCloseEvent>
#include "postframe.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setindex(int ind);
    QImage Convert_Mat_to_Qimg(cv::Mat fame);
    void setCameraIndex(int value);
    int getCameraIndex();
    void setCameraId();
    void AivailableCamerasNames();

    cv::Mat getOframe() const;

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_Enroll_triggered();
    void on_actionExit_2_triggered();
    void on_Identify_triggered();
    void on_start_clicked();
    void on_identify_clicked();
    void on_Enroll_2_clicked();
    void on_refresh_clicked();
    void on_Delete_clicked();

private:
    Ui::MainWindow *ui;
    QMessageBox qmb ;
    QGraphicsPixmapItem pixmap,px;
    cv::VideoCapture video;
    int cameraIndex;
    cv::Mat frame,Oframe;
    PostFrame pf;



};

#endif // MAINWINDOW_H
