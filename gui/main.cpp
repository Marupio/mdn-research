// gui/main.cpp
#include <QApplication>
#include "MainWindow.h"

#include <QLabel>

int main(int argc, char *argv[]) {
    // QApplication app(argc, argv);
    // QWidget window;
    // QLabel *label = new QLabel(QApplication::translate("windowlayout", "Name:"));
    // QLineEdit *lineEdit = new QLineEdit();

    // QHBoxLayout *layout = new QHBoxLayout();
    // layout->addWidget(label);
    // layout->addWidget(lineEdit);
    // window.setLayout(layout);
    // window.setWindowTitle(
    //     QApplication::translate("windowlayout", "Window layout"));
    // window.show();
    // return app.exec();

    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}

