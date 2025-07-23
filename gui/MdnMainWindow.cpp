#include "MdnMainWindow.h"
#include "ui_MdnMainWindow.h"

MdnMainWindow::MdnMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MdnMainWindow)
{
    ui->setupUi(this);
}

MdnMainWindow::~MdnMainWindow()
{
    delete ui;
}
