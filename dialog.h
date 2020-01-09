#ifndef DIALOG_H
#define DIALOG_H
#include <opencv2/opencv.hpp>
#include <QDialog>
#include <QGraphicsPixmapItem>


namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();
void setFrame(const cv::Mat &value);
    QImage Convert_Mat_to_Qimg(cv::Mat frame);
public:
    QGraphicsPixmapItem px;
    Ui::Dialog *ui;
    cv::Mat frame;

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();



//private:
  //  Ui::Dialog *ui;
};

#endif // DIALOG_H
