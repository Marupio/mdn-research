#pragma once

// This file was created during a prototyping exercise for the GUI.  It remains as a reference.

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
