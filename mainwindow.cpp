
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog.h"
#include "add_person.h"
#include<QMenuBar>
#include <QMediaService>
#include <QMediaRecorder>
#include <QLineEdit>
#include <QCameraInfo>
#include <QMediaMetaData>
#include <string>
#include <QMessageBox>
#include <QPalette>
#include "postframe.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QCheckBox>


Q_DECLARE_METATYPE(QCameraInfo)
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    pf=PostFrame();
    statusBar()->showMessage(tr("Ready"));
    ui->setupUi(this);
    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);
    ui->graphicsView_2->setScene(new QGraphicsScene(this));
    ui->graphicsView_2->scene()->addItem(&px);
    ui->refresh->setIcon(QIcon(":/images/icon/refresh.png"));
    ui->refresh->setIconSize(QSize(30, 30));
    ui->graphicsView->setStyleSheet("background-color: black");
    QMainWindow::setFixedHeight(661);
    QMainWindow::setFixedWidth(906);
    statusBar()->showMessage(tr("Ready"));
    AivailableCamerasNames();
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Enroll_triggered()
{
    qmb.setText("The document has been modified.");qmb.exec();

}

void MainWindow::on_actionExit_2_triggered()
{
    QApplication::quit();

}

void MainWindow::on_Identify_triggered()
{
    QMessageBox at;
    at.setText("The document has been modified.");
    at.exec();
}

void MainWindow::on_start_clicked()
{
    using namespace cv;
    // ui->textBrowser->clearHistory();
    // ui->graphicsView_2->resetMatrix();
    if(video.isOpened())
    {
        ui->start->setText("Start");
        video.release();
        return;
    }
    bool isCamera;
    setCameraId();
    if(isCamera)
    {
        if(!video.open(getCameraIndex()))
        {
            QMessageBox::critical(this,
                                  "Camera Error",
                                  "Make sure you entered a correct camera index,"
                                  "<br>or that the camera is not being accessed by another program!");
            return;
        }
    }
    else
    {
        if(!video.open(getCameraIndex()))
        {
            QMessageBox::critical(this,
                                  "Video Error",
                                  "Make sure you entered a correct and supported video file path,"
                                  "<br>or a correct RTSP feed URL!");
            return;
        }
    }
    ui->start->setText("Stop");
    while(video.isOpened())
    {
        video >> frame;
        Oframe=frame.clone();
        pf.detectFaceOpenCVHaar(frame,300,0);
        if(!frame.empty())
        {
            pixmap.setPixmap( QPixmap::fromImage(Convert_Mat_to_Qimg(frame).rgbSwapped()) );
            ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);
        }
        qApp->processEvents();
    }
    ui->start->setText("Start");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(video.isOpened())
    {
        QMessageBox::warning(this,
                             "Warning",
                             "Stop the video before closing the application!");
        event->ignore();
    }
    else
    {
        event->accept();
    }
}
void MainWindow::on_identify_clicked()
{
    if(!frame.empty())
    {
        px.setPixmap( QPixmap::fromImage(Convert_Mat_to_Qimg(frame).rgbSwapped()) );
        ui->graphicsView_2->fitInView(&px, Qt::KeepAspectRatio);
        pf.setFrame(frame);
        string name="TELNET AI : WELCOME "+pf.put_frame("/users/identify");
        ui->textBrowser->setPlainText(QString::fromStdString(name));
    }
}

QImage MainWindow::Convert_Mat_to_Qimg(cv::Mat frame)
{
    QImage qimg(frame.data,
                frame.cols,
                frame.rows,
                frame.step,
                QImage::Format_RGB888);
    return qimg;
}

void MainWindow::on_Enroll_2_clicked()
{
    Add_Person add;
    add.setModal(true);
    add.setFrame(Oframe);
    add.exec();
}

void MainWindow::on_refresh_clicked()
{
    ui->menuDevices_2->clear();
    ui->AllCam->clear();
    AivailableCamerasNames();
}

int MainWindow::getCameraIndex()
{
    return cameraIndex;
}

void MainWindow::setCameraIndex(int value)
{
    cameraIndex = value;
}
void MainWindow::setCameraId()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    setCameraIndex(cameras.at(ui->AllCam->currentIndex()).deviceName().at(10).digitValue());
}
void MainWindow::AivailableCamerasNames()
{
    const QList<QCameraInfo> availableCameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, availableCameras)
    {
        ui->AllCam->addItem(cameraInfo.description());
        ui->menuDevices_2->addAction(cameraInfo.description());
    }
}

void MainWindow::on_Delete_clicked()
{
    Dialog dia;
    dia.setModal(true);
    dia.setFrame(Oframe);
    dia.exec();

}

cv::Mat MainWindow::getOframe() const
{
    return Oframe;
}
