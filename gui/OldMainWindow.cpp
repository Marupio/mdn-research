// gui/MainWindow.cpp
#include "MainWindow.h"
#include "NumberDisplayWidget.h"
#include "../library/Coord.h"

#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 1. Create a central widget and layout
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central); // layout for central widget

    // 2. Create the upper UI elements (e.g., label and line edit)
    QLabel *label = new QLabel("Name:");
    QLineEdit *lineEdit = new QLineEdit();
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(label);
    topLayout->addWidget(lineEdit);

    // Optional: add a separator line
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);



    // 3. Create the tab widget and add your tabs
    m_tabWidget = new QTabWidget(this);
    QWidget *mainTab = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(mainTab);
    layout->addWidget(new QLabel("This is the main tab content."));
    mainTab->setLayout(layout);
    m_tabWidget->addTab(mainTab, "Main Tab");

    NumberDisplayWidget* display = new NumberDisplayWidget();
    m_mdnMap[m_nextMdnId] = new mdn::Mdn2d();
    display->setModel(m_mdnMap[m_nextMdnId++]);
    m_tabWidget->addTab(display, "Display");

    // 4. Assemble everything into the main layout
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(line);
    mainLayout->addWidget(m_tabWidget);

    // 5. Apply the assembled widget as central
    setCentralWidget(central);
    setWindowTitle("Window layout");

    //     // m_tabWidget = new QTabWidget(this);
    //     // setCentralWidget(m_tabWidget);
    //     //
    //     // createMDNTab(m_nextMdnId++);
    //
    //     m_tabWidget = new QTabWidget(this);
    //     setCentralWidget(m_tabWidget);
    //     setWindowTitle(
    //         QApplication::translate("windowlayout", "Window layout"));
    //     QLabel *label = new QLabel(QApplication::translate("windowlayout", "Name:"));
    //     QLineEdit *lineEdit = new QLineEdit();
    //     QWidget *mainTab = new QWidget(this);
    //     QHBoxLayout *layout = new QHBoxLayout(mainTab);
    //     layout->addWidget(label);
    //     layout->addWidget(lineEdit);
    //     mainTab->setLayout(layout);
    //     m_tabWidget->addTab(mainTab, "Main Tab");
    //
    //     NumberDisplayWidget* display = new NumberDisplayWidget();
    //     m_mdnMap[m_nextMdnId] = new mdn::Mdn2d();
    //     display->setModel(m_mdnMap[m_nextMdnId++]);
    //     m_tabWidget->addTab(display, "Display");
    //
    //     // QLabel *label = new QLabel(QApplication::translate("windowlayout", "Name:"));
    //     // QLineEdit *lineEdit = new QLineEdit();
    //     // QWidget *central = new QWidget(this);
    //     // QHBoxLayout *layout = new QHBoxLayout(central);
    //     // layout->addWidget(label);
    //     // layout->addWidget(lineEdit);
    //     // central->setLayout(layout);
    //     // setWindowTitle(
    //     //     QApplication::translate("windowlayout", "Window layout"));
    //     // setCentralWidget(central);
    //
    //
    //
    //     // QWidget *central = new QWidget(this);
    //     // QVBoxLayout *layout = new QVBoxLayout(central);
    //
    //     // QPushButton *newMDNButton = new QPushButton("New MDN Tab");
    //     // connect(newMDNButton, &QPushButton::clicked, this, &MainWindow::addNewMDN);
    //
    //     // QPushButton *addValueButton = new QPushButton("Add (0,0) +1 to Current Tab");
    //     // connect(addValueButton, &QPushButton::clicked, this, &MainWindow::addValueToCurrent);
    //
    //     // m_tabWidget = new QTabWidget;
    //     // m_tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    //     // connect(m_tabWidget, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::onTabBarDoubleClicked);
    //     // connect(
    //     //     m_tabWidget,
    //     //     &QTabWidget::customContextMenuRequested,
    //     //     this,
    //     //     &MainWindow::onTabContextMenuRequested
    //     // );
    //
    //     // layout->addWidget(newMDNButton);
    //     // layout->addWidget(addValueButton);
    //     // layout->addWidget(m_tabWidget);
    //     // setCentralWidget(central);
    //
    //     // addNewMDN(); // create initial tab
}

MainWindow::~MainWindow() {
    qDeleteAll(m_mdnMap);
}

void MainWindow::createMDNTab(int id) {
    QTextEdit* textView = new QTextEdit;
    textView->setReadOnly(true);
    m_tabWidget->addTab(textView, QString("MDN%1").arg(id));
    m_mdnMap[id] = new mdn::Mdn2d();
    NumberDisplayWidget* display = new NumberDisplayWidget(this);
    display->setModel(m_mdnMap[id]);
    // setCentralWidget(display);
}

void MainWindow::addNewMDN() {
    createMDNTab(m_nextMdnId++);
}

void MainWindow::addValueToCurrent() {
    int index = m_tabWidget->currentIndex();
    if (index < 0) return;

    QTextEdit* textView = qobject_cast<QTextEdit*>(m_tabWidget->currentWidget());
    if (!textView) return;

    mdn::Mdn2d* mdn = m_mdnMap[index];
    mdn->add(mdn::COORD_ORIGIN, 1);
    textView->setText(QString::fromStdString(mdn->toString()));

}

void MainWindow::renameTab() {
    int index = m_tabWidget->currentIndex();
    if (index < 0) return;
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Tab", "New name:", QLineEdit::Normal, m_tabWidget->tabText(index), &ok);
    if (ok && !newName.isEmpty()) {
        m_tabWidget->setTabText(index, newName);
    }
}

void MainWindow::duplicateTab() {
    int index = m_tabWidget->currentIndex();
    if (index < 0) return;

    QTextEdit* oldTextView = qobject_cast<QTextEdit*>(m_tabWidget->currentWidget());
    if (!oldTextView) return;

    mdn::Mdn2d* original = m_mdnMap[index];
    mdn::Mdn2d* copy = new mdn::Mdn2d(*original);
    m_mdnMap[m_nextMdnId] = copy;

    NumberDisplayWidget* newDisplay = new NumberDisplayWidget(this);
    newDisplay->setModel(m_mdnMap[m_nextMdnId]);
    m_tabWidget->addTab(newDisplay, QString("MDN%1").arg(m_nextMdnId++));
}

void MainWindow::deleteTab() {
    int index = m_tabWidget->currentIndex();
    if (index < 0) return;

    QWidget* widget = m_tabWidget->widget(index);
    m_tabWidget->removeTab(index);
    delete widget;
    delete m_mdnMap.take(index);
}

void MainWindow::onTabBarDoubleClicked(int index) {
    if (index >= 0) renameTab();
}

void MainWindow::onTabContextMenuRequested(const QPoint &pos) {
    QMenu contextMenu;
    contextMenu.addAction("Rename", this, &MainWindow::renameTab);
    contextMenu.addAction("Duplicate", this, &MainWindow::duplicateTab);
    contextMenu.addAction("Delete", this, &MainWindow::deleteTab);
    contextMenu.exec(m_tabWidget->mapToGlobal(pos));
}
