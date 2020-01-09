#include "dialog.h"
#include "ui_dialog.h"
#include "postframe.h"


PostFrame ps;
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    std::vector<string> names;
    names=ps.getListName();
    for(int i=0;i<names.size();i++)
    {
        ui->comboBox->addItem( QString::fromLocal8Bit(names.at(i).c_str()));
    }
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_clicked()
{
    this->close();
}


void Dialog::on_pushButton_2_clicked()
{
    ps.setLabel(ui->comboBox->currentText().toStdString());
    ps.put_frame("/users/delete");
    this->close();
}
