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
    tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabWidget, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::onTabBarDoubleClicked);
    connect(tabWidget, &QTabWidget::customContextMenuRequested, this, &MainWindow::onTabContextMenuRequested);

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
    mdnMap[id] = new mdn::PlaceHolderMdn(10);
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

    mdn::PlaceHolderMdn* mdn = mdnMap[index];
    mdn->addValueAt(0, 0, 1);
    textView->setText(QString::fromStdString(mdn->toString()));
}

void MainWindow::renameTab() {
    int index = tabWidget->currentIndex();
    if (index < 0) return;
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Tab", "New name:", QLineEdit::Normal, tabWidget->tabText(index), &ok);
    if (ok && !newName.isEmpty()) {
        tabWidget->setTabText(index, newName);
    }
}

void MainWindow::duplicateTab() {
    int index = tabWidget->currentIndex();
    if (index < 0) return;

    QTextEdit* oldTextView = qobject_cast<QTextEdit*>(tabWidget->currentWidget());
    if (!oldTextView) return;

    mdn::PlaceHolderMdn* original = mdnMap[index];
    mdn::PlaceHolderMdn* copy = new mdn::PlaceHolderMdn(*original);
    QTextEdit* newTextView = new QTextEdit;
    newTextView->setReadOnly(true);
    newTextView->setText(QString::fromStdString(copy->toString()));
    mdnMap[nextMDNId] = copy;
    tabWidget->addTab(newTextView, QString("MDN%1").arg(nextMDNId++));
}

void MainWindow::deleteTab() {
    int index = tabWidget->currentIndex();
    if (index < 0) return;

    QWidget* widget = tabWidget->widget(index);
    tabWidget->removeTab(index);
    delete widget;
    delete mdnMap.take(index);
}

void MainWindow::onTabBarDoubleClicked(int index) {
    if (index >= 0) renameTab();
}

void MainWindow::onTabContextMenuRequested(const QPoint &pos) {
    QMenu contextMenu;
    contextMenu.addAction("Rename", this, &MainWindow::renameTab);
    contextMenu.addAction("Duplicate", this, &MainWindow::duplicateTab);
    contextMenu.addAction("Delete", this, &MainWindow::deleteTab);
    contextMenu.exec(tabWidget->mapToGlobal(pos));
}
