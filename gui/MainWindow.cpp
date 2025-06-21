// gui/MainWindow.cpp
#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);

    QPushButton *newMDNButton = new QPushButton("New MDN Tab");
    connect(newMDNButton, &QPushButton::clicked, this, &MainWindow::addNewMDN);

    QPushButton *addValueButton = new QPushButton("Add (0,0) +1 to Current Tab");
    connect(addValueButton, &QPushButton::clicked, this, &MainWindow::addValueToCurrent);

    tabWidget = new QTabWidget;

    layout->addWidget(newMDNButton);
    layout->addWidget(addValueButton);
    layout->addWidget(tabWidget);
    setCentralWidget(central);

    addNewMDN(); // create initial tab
}

MainWindow::~MainWindow() {
    qDeleteAll(mdnMap);
}

void MainWindow::createMDNTab(int id) {
    QTextEdit* textView = new QTextEdit;
    textView->setReadOnly(true);
    tabWidget->addTab(textView, QString("MDN%1").arg(id));
    mdnMap[id] = new MultiDimensionalNumber(10);
    textView->setText(QString::fromStdString(mdnMap[id]->toString()));
}

void MainWindow::addNewMDN() {
    createMDNTab(nextMDNId++);
}

void MainWindow::addValueToCurrent() {
    int index = tabWidget->currentIndex();
    if (index < 0) return;

    QTextEdit* textView = qobject_cast<QTextEdit*>(tabWidget->currentWidget());
    if (!textView) return;

    MultiDimensionalNumber* mdn = mdnMap[index];
    mdn->addValueAt(0, 0, 1);
    textView->setText(QString::fromStdString(mdn->toString()));
}
