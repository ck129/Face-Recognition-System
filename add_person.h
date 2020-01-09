#ifndef ADD_PERSON_H
#define ADD_PERSON_H
#include <opencv2/opencv.hpp>
#include <QDialog>
#include <QGraphicsPixmapItem>
namespace Ui {
class Add_Person;
}

class Add_Person : public QDialog
{
    Q_OBJECT

public:

    explicit Add_Person(QWidget *parent = nullptr);
    ~Add_Person();

    void setFrame(const cv::Mat &value);
    QImage Convert_Mat_to_Qimg(cv::Mat frame);
public:
    QGraphicsPixmapItem px;
    Ui::Add_Person *ui;
    cv::Mat frame;
private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
};

#endif // ADD_PERSON_H
