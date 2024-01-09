#include "listrank.h"
#include "ui_listrank.h"

listrank::listrank(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::listrank)
{
    ui->setupUi(this);
}

listrank::~listrank()
{
    delete ui;
}
