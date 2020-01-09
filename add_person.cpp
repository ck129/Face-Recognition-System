#include "add_person.h"
#include "ui_add_person.h"
#include "postframe.h"
#include <QMessageBox>



Add_Person::Add_Person(QWidget *parent ) :
    QDialog(parent),
    ui(new Ui::Add_Person)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&px);

}

Add_Person::~Add_Person()
{
    delete ui;
}

void Add_Person::setFrame(const cv::Mat &value)
{
    frame = value;
    cv::Mat face;
    cv::resize(frame,face,cv::Size(140, 142));
    px.setPixmap( QPixmap::fromImage(Convert_Mat_to_Qimg(face).rgbSwapped()) );

}
QImage Add_Person::Convert_Mat_to_Qimg(cv::Mat frame)
{
    QImage qimg(frame.data,
                frame.cols,
                frame.rows,
                frame.step,
                QImage::Format_RGB888);
    return qimg;
}

void Add_Person::on_pushButton_2_clicked()
{
    PostFrame pf;
    pf.setLabel(ui->lineEdit->displayText().toStdString());
    pf.setFrame(frame);
    string response=pf.put_frame("/users/Add");
    QMessageBox msgBox;
    msgBox.setText(QString::fromStdString(response));
    int ret = msgBox.exec();
    if(ret==QMessageBox::Ok){this->close();}
}

void Add_Person::on_pushButton_clicked()
{
    this->close();
}
