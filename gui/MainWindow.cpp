#include "MainWindow.hpp"

#include "NumberDisplayWidget.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_project(new mdn::Project(this))
{
    createMenus();
    setupLayout();
    setWindowTitle("MDN Editor");
}


void MainWindow::createMenus() {
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("New Project", this, &MainWindow::newProjectRequested);
    fileMenu->addAction("New Mdn2d", this, &MainWindow::newMdn2dRequested);
    fileMenu->addSeparator();
    fileMenu->addAction("Open Project", this, &MainWindow::openProjectRequested);
    fileMenu->addAction("Open Mdn2d", this, &MainWindow::openMdn2dRequested);
    fileMenu->addSeparator();
    fileMenu->addAction("Save Project", this, &MainWindow::saveProjectRequested);
    fileMenu->addAction("Save Mdn2d", this, &MainWindow::saveMdn2dRequested);
    fileMenu->addSeparator();
    fileMenu->addAction("Close Project", this, &MainWindow::closeProjectRequested);
    fileMenu->addAction("Exit", this, &MainWindow::close);

    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("Undo");
    editMenu->addAction("Redo");
    editMenu->addSeparator();
    editMenu->addAction("Select All");
    editMenu->addAction("Select Row");
    editMenu->addAction("Select Column");
    editMenu->addAction("Select Box");
    editMenu->addSeparator();
    editMenu->addAction("Properties");
    editMenu->addSeparator();
    editMenu->addAction("Copy");
    editMenu->addAction("Cut");
    editMenu->addAction("Paste");
    editMenu->addSeparator();
    editMenu->addAction("Delete");

    QMenu* toolsMenu = menuBar()->addMenu("&Tools");
    // Placeholder for future items

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("Get Help");
    helpMenu->addAction("Donate");
    helpMenu->addAction("About");
}

void MainWindow::setupLayout() {
    m_splitter = new QSplitter(Qt::Vertical, this);

    // Top half - Mdn2d tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::South);

    // For now, Project will be held by MainWindow
    createNewProject();

    // Create one dummy tab to start
    m_display = new NumberDisplayWidget(this);

    m_display->setProject(m_project);
    m_display->setModel(m_project->activeMdn());

    auto* mdn = new mdn::Mdn2d();
    m_display->setModel(mdn);



    m_tabWidget->addTab(m_display, QString("Mdn%1").arg(m_nextMdnId++));
    m_mdnMap[m_nextMdnId] = mdn;
    m_tabWidget->addTab(m_display, QString("Mdn%1").arg(m_nextMdnId++));

    // Bottom half - command history + input
    QWidget* commandPane = new QWidget(this);
    QVBoxLayout* cmdLayout = new QVBoxLayout(commandPane);
    m_commandHistory = new QTextEdit(commandPane);
    m_commandHistory->setReadOnly(true);
    QHBoxLayout* inputLayout = new QHBoxLayout();
    m_commandInput = new QLineEdit(commandPane);
    m_submitButton = new QPushButton("Submit", commandPane);
    m_copyButton = new QPushButton("Copy", commandPane);
    inputLayout->addWidget(m_commandInput);
    inputLayout->addWidget(m_submitButton);
    inputLayout->addWidget(m_copyButton);
    cmdLayout->addWidget(m_commandHistory);
    cmdLayout->addLayout(inputLayout);

    m_splitter->addWidget(m_tabWidget);
    m_splitter->addWidget(commandPane);
    m_splitter->setStretchFactor(0, 3);
    m_splitter->setStretchFactor(1, 1);

    setCentralWidget(m_splitter);
}


void MainWindow::createNewProject() {
    if (m_project) {
        delete m_project;
        m_project = nullptr;
    }
    m_project = new mdn::Project(this);

}


void MainWindow::updateTabs() {
    if (!m_project) {
        // Nothing to do
        return;
    }

    for (auto& [idx, pair] : project->data()) {
        const auto& mdn = pair.first;
        auto* w = new NumberDisplayWidget;
        w->setModel(&mdn, &pair.second);              // your setter; bind read-only model + selection
        w->setProject(project);                        // if you call back for cursor ops
        int tab = m_tabWidget->addTab(w, QString::fromStdString(mdn.name()));
        // Optional: remember idx on the widget so you can get back to Project’s index later
        w->setProperty("projIndex", idx);
    }

}
