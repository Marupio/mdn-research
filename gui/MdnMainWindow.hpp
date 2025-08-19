#ifndef MDNMAINWINDOW_H
#define MDNMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MdnMainWindow;
}

class MdnMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MdnMainWindow(QWidget *parent = nullptr);
    ~MdnMainWindow();

private:
    Ui::MdnMainWindow *ui;
};

#endif // MDNMAINWINDOW_H
