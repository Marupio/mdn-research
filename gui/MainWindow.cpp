#include "MainWindow.hpp"

#include <QApplication>
#include <QEvent>
#include <QInputDialog>
#include <QList>
#include <QMenuBar>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>

#include "OpsController.hpp"
#include "MdnQtInterface.hpp"
#include "NumberDisplayWidget.hpp"
#include "../library/Logger.hpp"

mdn::gui::MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_project(nullptr)
{
    Log_Debug2_H("Constructing MainWindow")
    createSplitter();
    createMenus();
    m_command = new CommandWidget(this);
    setupLayout();
    setWindowTitle("MDN Editor");
    Log_Debug2_H("");
}


void mdn::gui::MainWindow::onTabContextMenu(const QPoint& pos)
{
    Log_Debug3_H("");
    QTabBar* bar = m_tabWidget->tabBar();
    const int index = bar->tabAt(pos);
    if (index < 0) {
        Log_Debug3_T("");
        return;
    }
    QWidget* tabPage = m_tabWidget->widget(index);
    auto* view = qobject_cast<NumberDisplayWidget*>(tabPage);
    if (!view) {
        Log_Debug3_T("");
        return;
    }
    QMenu menu(this);
    QAction* actRename    = menu.addAction("Rename");
    QAction* actDuplicate = menu.addAction("Duplicate");
    QAction* actCopy      = menu.addAction("Copy");
    QAction* actPaste     = menu.addAction("Paste");
    // QAction* actMoveLeft  = menu.addAction("Move Left");
    // QAction* actMoveRight = menu.addAction("Move Right");
    menu.addSeparator();
    QAction* actDelete    = menu.addAction("Delete");

    // Enable/disable niceties
    // actMoveLeft->setEnabled(index > 0);
    // actMoveRight->setEnabled(index < m_tabWidget->count() - 1);
    // Paste stays enabled; your Project::pasteOnSelection() validates scope and will warn if invalid.

    QAction* picked = menu.exec(bar->mapToGlobal(pos));
    if (!picked) {
        Log_Debug3_T("");
        return;
    }
    if (picked == actRename) {
        renameTab(index);
    } else if (picked == actDuplicate) {
        duplicateTab(index);
    } else if (picked == actCopy) {
        m_project->copyMdn(index);
    } else if (picked == actPaste) {
        pasteTab(index + 1);               // insert after current tab
    // } else if (picked == actMoveLeft) {
    //     bool moveMdn(int fromIndex, int toIndex);
    //     m_project->moveMdn(index, index-1);
    //     // const int to = std::max(0, index - 1);
    //     // if (to != index) onTabMoved(index, to);  // reuse your mover
    //     // // QTabWidget move for UI:
    //     // m_tabWidget->tabBar()->moveTab(index, to);
    // } else if (picked == actMoveRight) {
    //     const int to = std::min(index + 1, m_tabWidget->count() - 1);
    //     if (to != index) onTabMoved(index, to);
    //     m_tabWidget->tabBar()->moveTab(index, to);
    } else if (picked == actDelete) {
        onTabCloseRequested(index);           // reuse your close handler
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::onProjectTabsAboutToChange()
{
    Log_Debug2_H("");

    if (!m_tabWidget) {
        Log_Debug2_T("");
        return;
    }

    QSignalBlocker blockTabs(m_tabWidget);

    if (centralWidget()) {
        centralWidget()->setUpdatesEnabled(false);
    }

    const int n = m_tabWidget->count();
    for (int i = 0; i < n; ++i) {
        auto* view = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(i));
        if (!view) {
            continue;
        }
        view->cancelCellEdit();
    }

    Log_Debug2_T("");
}


void mdn::gui::MainWindow::onProjectTabsChanged(int currentIndex)
{
    Log_Debug2_H("currentIndex=" << currentIndex);

    if (!m_tabWidget || !m_project) {
        Log_Debug2_T("");
        return;
    }

    QSignalBlocker blockTabs(m_tabWidget);

    const int n = m_tabWidget->count();
    for (int i = n - 1; i >= 0; --i) {
        QWidget* w = m_tabWidget->widget(i);
        m_tabWidget->removeTab(i);
        delete w;
    }

    createTabs();

    int idx = currentIndex;
    const int count = m_tabWidget->count();
    if (idx < 0) {
        idx = 0;
    } else {
        if (idx >= count) {
            idx = count - 1;
        }
    }
    if (count > 0) {
        m_tabWidget->setCurrentIndex(idx);
    }

    if (centralWidget()) {
        centralWidget()->setUpdatesEnabled(true);
    }

    if (m_ops) {
        m_ops->refreshTabNames();
    }

    focusActiveGrid();

    Log_Debug2_T("");
}


void mdn::gui::MainWindow::onSplitterMoved(int pos, int index) {
    QList<int> sizes = m_splitter->sizes();
    int total = 0;
    for (int s : sizes) {
        total += s;
    }
    if (total > 0 && sizes.size() >= 2) {
        m_splitRatio = double(sizes[0]) / double(total);
    }
}


void mdn::gui::MainWindow::onCommandSubmitted(const QString& text) {
    m_command->appendLine(QStringLiteral("» %1").arg(text));
}


void mdn::gui::MainWindow::onOpsPlan(const OpsController::Plan& p) {
    if (!m_project) {
        return;
    }

    Mdn2d* a = m_project->getMdn(p.indexA, true);
    Mdn2d* b = m_project->getMdn(p.indexB, true);

    if (a == nullptr) {
        return;
    }
    if (b == nullptr) {
        return;
    }

    if (p.dest == OpsController::Dest::InPlace) {
        Mdn2d ans(m_project->config(), a->name());
        if (p.op == OpsController::Op::Add) {
            a->plus(*b, ans);
        } else {
            if (p.op == OpsController::Op::Subtract) {
                a->minus(*b, ans);
            } else {
                if (p.op == OpsController::Op::Multiply) {
                    a->multiply(*b, ans);
                } else {
                    ans = Mdn2d(m_project->config(), a->name());
                    a->divide(*b, ans, Fraxis::Default);
                }
            }
        }
        *a = ans;
        syncTabsToProject();
        m_tabWidget->setCurrentIndex(p.indexA);
        if (m_ops) {
            m_ops->refreshTabNames();
        }
        return;
    }

    if (p.dest == OpsController::Dest::ToNew) {
        std::string requestedName = MdnQtInterface::fromQString(p.newName);
        if (requestedName.empty()) {
            requestedName = std::string("Result");
        }

        Mdn2d ans(m_project->config(), requestedName);
        if (p.op == OpsController::Op::Add) {
            a->plus(*b, ans);
        } else {
            if (p.op == OpsController::Op::Subtract) {
                a->minus(*b, ans);
            } else {
                if (p.op == OpsController::Op::Multiply) {
                    a->multiply(*b, ans);
                } else {
                    ans = Mdn2d(m_project->config(), requestedName);
                    a->divide(*b, ans, Fraxis::Default);
                }
            }
        }

        int insertAt = p.indexA + 1;
        m_project->insertMdn(ans, insertAt);

        const auto& nameToIndex = m_project->data_addressingNameToIndex();
        auto it = nameToIndex.find(ans.name());
        int actualIndex = -1;
        if (it != nameToIndex.end()) {
            actualIndex = it->second;
        }

        if (actualIndex < 0) {
            actualIndex = m_tabWidget->count();
        }

        createTabForIndex(actualIndex);
        m_tabWidget->setCurrentIndex(actualIndex);
        if (m_ops) {
            m_ops->refreshTabNames();
        }
        return;
    }
}


void mdn::gui::MainWindow::slotSelectNextTab()
{
    Log_Debug3_H("slotSelectNextTab");
    if (!m_tabWidget) {
        Log_Debug3_T("");
        return;
    }
    const int idx = m_tabWidget->currentIndex();
    const int count = m_tabWidget->count();
    if (idx + 1 < count) {
        m_tabWidget->setCurrentIndex(idx + 1);
        focusActiveGrid();
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::slotSelectPrevTab()
{
    Log_Debug3_H("slotSelectPrevTab");
    if (!m_tabWidget) {
        Log_Debug3_T("");
        return;
    }
    const int idx = m_tabWidget->currentIndex();
    if (idx - 1 >= 0) {
        m_tabWidget->setCurrentIndex(idx - 1);
        focusActiveGrid();
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::slotMoveTabRight()
{
    Log_Debug3_H("slotMoveTabRight");
    if (!m_tabWidget) {
        Log_Debug3_T("");
        return;
    }
    const int idx = m_tabWidget->currentIndex();
    const int count = m_tabWidget->count();
    if (idx >= 0 && idx + 1 < count) {
        QTabBar* bar = m_tabWidget->tabBar();
        bar->moveTab(idx, idx + 1);
        m_tabWidget->setCurrentIndex(idx + 1);
        focusActiveGrid();
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::slotMoveTabLeft()
{
    Log_Debug3_H("slotMoveTabLeft");
    if (!m_tabWidget) {
        Log_Debug3_T("");
        return;
    }
    const int idx = m_tabWidget->currentIndex();
    if (idx > 0) {
        QTabBar* bar = m_tabWidget->tabBar();
        bar->moveTab(idx, idx - 1);
        m_tabWidget->setCurrentIndex(idx - 1);
        focusActiveGrid();
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::slotDebugShowAllTabs()
{
    Log_Debug3_H("");
    if (m_project)
    {
        std::ostringstream oss;
        m_project->debugShowAllTabs(oss);
        Log_Info(oss.str());
    }
    Log_Debug3_T("");
}


bool mdn::gui::MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_splitter) {
        if (event->type() == QEvent::Resize) {
            applySplitRatio();
        }
    }
    if (event->type() == QEvent::FocusIn) {
        QWidget* w = qobject_cast<QWidget*>(watched);

        if (w != nullptr) {
            if (w->window() == this) {
                if (!isFocusAllowedWidget(w)) {
                    focusActiveGrid();
                    return true;
                }
            }
        }
    }

    if (event->type() == QEvent::WindowActivate) {
        focusActiveGrid();
    }

    return QMainWindow::eventFilter(watched, event);
}


void mdn::gui::MainWindow::createSplitter() {
    m_splitter = new QSplitter(Qt::Vertical, this);
    setCentralWidget(m_splitter);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setHandleWidth(6);
    m_splitter->installEventFilter(this);
    connect(m_splitter, &QSplitter::splitterMoved, this, &MainWindow::onSplitterMoved);
    m_splitRatio = 0.5;
}


void mdn::gui::MainWindow::createMenus() {
    Log_Debug3_H("");
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("New Project", this, &mdn::gui::MainWindow::newProjectRequested);
    fileMenu->addAction("New Mdn2d", this, &mdn::gui::MainWindow::newMdn2dRequested);
    fileMenu->addSeparator();
    fileMenu->addAction("Open Project", this, &mdn::gui::MainWindow::openProjectRequested);
    fileMenu->addAction("Open Mdn2d", this, &mdn::gui::MainWindow::openMdn2dRequested);
    fileMenu->addSeparator();
    fileMenu->addAction("Save Project", this, &mdn::gui::MainWindow::saveProjectRequested);
    fileMenu->addAction("Save Mdn2d", this, &mdn::gui::MainWindow::saveMdn2dRequested);
    fileMenu->addSeparator();
    fileMenu->addAction("Close Project", this, &mdn::gui::MainWindow::closeProjectRequested);
    fileMenu->addAction("Exit", this, &mdn::gui::MainWindow::close);

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
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::setupLayout() {
    Log_Debug3_H("")

    // Top half - Mdn2d tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::South);
    auto* bar = m_tabWidget->tabBar();
    bar->setMovable(true);
    connect(
        bar,                        // sender
        &QTabBar::tabMoved,         // signal
        this,                       // receiver
        &mdn::gui::MainWindow::onTabMoved     // method
    );
    bar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(
        bar,
        &QTabBar::customContextMenuRequested,
        this,
        &mdn::gui::MainWindow::onTabContextMenu
    );

    // For now, Project will be held by MainWindow
    Log_Debug3("Dispatch - createNewProject");
    createNewProject();
    Log_Debug3("Dispatch - createTabs");
    createTabs();

    // Bottom half - command history + input
    m_splitter->addWidget(m_tabWidget);
    m_splitter->addWidget(m_command);
    m_splitter->setStretchFactor(0, 3);
    m_splitter->setStretchFactor(1, 1);

    setCentralWidget(m_splitter);

    Log_Debug3("Dispatch - initOperationsUi");
    initOperationsUi();

    Log_Debug3("Dispatch - initFocusModel");
    initFocusModel();

    Log_Debug3_T("");
}


void mdn::gui::MainWindow::createNewProject() {
    Log_Debug3_H("");
    if (m_project) {
        delete m_project;
        m_project = nullptr;
    }
    m_project = new Project(this);
    if (m_project) {
        connect(m_project, &mdn::gui::Project::tabsAboutToChange,
                this, &mdn::gui::MainWindow::onProjectTabsAboutToChange);
        connect(m_project, &mdn::gui::Project::tabsChanged,
                this, &mdn::gui::MainWindow::onProjectTabsChanged);
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::createTabs() {
    Log_Debug3_H("");
    {
        mdn::Logger& loginst = mdn::Logger::instance();
        std::string dbind = loginst.debug_indentenators();
        loginst.info(dbind);
    }
    if (!m_project) {
        // Nothing to do
        Log_Debug3_T("No project");
        return;
    }

    const std::unordered_map<int, std::string>& tabNames(m_project->data_addressingIndexToName());
    std::vector<std::string> names(m_project->toc());
    for (int index = 0; index < names.size(); ++index) {
        Mdn2d* src = m_project->getMdn(index, true);
        Selection* sel = m_project->getSelection(index);
        QString qname = MdnQtInterface::toQString(names[index]);
        auto* ndw = new NumberDisplayWidget;
        ndw->setProject(m_project);
        ndw->setFocusPolicy(Qt::StrongFocus);
        ndw->setModel(src, sel);
        int tab = m_tabWidget->addTab(ndw, qname);
        connect(
            ndw,
            &mdn::gui::NumberDisplayWidget::requestSelectNextTab,
            this,
            &mdn::gui::MainWindow::slotSelectNextTab
        );
        connect(
            ndw,
            &mdn::gui::NumberDisplayWidget::requestSelectPrevTab,
            this,
            &mdn::gui::MainWindow::slotSelectPrevTab
        );
        connect(
            ndw,
            &mdn::gui::NumberDisplayWidget::requestMoveTabRight,
            this,
            &mdn::gui::MainWindow::slotMoveTabRight
        );
        connect(
            ndw,
            &mdn::gui::NumberDisplayWidget::requestMoveTabLeft,
            this,
            &mdn::gui::MainWindow::slotMoveTabLeft
        );
        connect(
            ndw,
            &mdn::gui::NumberDisplayWidget::requestDebugShowAllTabs,
            this,
            &mdn::gui::MainWindow::slotDebugShowAllTabs
        );
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::createTabForIndex(int index) {
    std::pair<Mdn2d, Selection>* entry = m_project->at(index);
    if (entry == nullptr) {
        return;
    }

    auto* view = new NumberDisplayWidget;
    view->setProject(m_project);
    view->setModel(&entry->first, &entry->second);

    QString tabName = QString::fromStdString(entry->first.name());
    int tab = m_tabWidget->insertTab(index, view, tabName);
    m_tabWidget->setCurrentIndex(tab);
    view->setFocus();
}


void mdn::gui::MainWindow::initOperationsUi() {
    int idx = m_splitter->indexOf(m_command);
    m_ops = new OpsController(this, m_tabWidget, m_command, this);
    QWidget* container = m_ops->bottomContainer();
    if (idx >= 0) {
        m_splitter->replaceWidget(idx, container);
    } else {
        m_splitter->addWidget(container);
    }

    connect(m_ops, &OpsController::planReady, this, &MainWindow::onOpsPlan);
    connect(m_tabWidget, &QTabWidget::currentChanged, m_ops, &OpsController::refreshTabNames);

    m_ops->refreshTabNames();
    connect(m_command, &CommandWidget::submitCommand, this, &MainWindow::onCommandSubmitted);
}


void mdn::gui::MainWindow::onTabMoved(int from, int to) {
    Log_Debug3_H("from " << from << " to " << to);
    m_project->moveMdn(from, to);
    syncTabsToProject();
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::onTabCloseRequested(int index) {
    Log_Debug3_H("");
    if (!m_project) {
        Log_Debug3_T("No project");
        return;
    }
    m_project->deleteMdn(index);
    if (m_ops) {
        m_ops->refreshTabNames();
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::renameTab(int index) {
    Log_Debug3_H("index=" << index);
    const std::string origName = m_project->nameOfMdn(index);
    QString origNameQ = MdnQtInterface::toQString(origName);

    bool ok = false;
    QString newNameQ = QInputDialog::getText(
        this,
        "Rename",
        "MDN name:",
        QLineEdit::Normal,
        origNameQ,
        &ok
    );
    if (!ok) {
        Log_Debug3_T("User changed their mind");
        return;
    }
    std::string newName = MdnQtInterface::fromQString(newNameQ);

    if (newName == origName) {
        Log_Debug3_T("Nothing to do");
        return;
    }

    std::string approvedName = m_project->renameMdn(index, newName);
    if (approvedName == origName) {
        Log_InfoQ("Failed to change name from '" << origName << "' to '" << newName << "'");
        Log_Debug3_T("");
        return;
    }
    QString approvedNameQ = MdnQtInterface::toQString(approvedName);
    m_tabWidget->setTabText(index, approvedNameQ);

    // Name changed, update
    if (m_ops) {
        m_ops->refreshTabNames();
    }

    if (approvedName == newName) {
        Log_Debug3_T("User got what they expect, say no more");
        return;
    }

    // Name changed, but not to what user asked for - this warrants a text box
    Log_InfoQ(
        "Could not change '" << origName << "' to '" << newName << "', name already exists. "
        << "Using '" << approvedName << "' instead."
    );
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::duplicateTab(int index)
{
    Log_Debug3_H("index=" << index);
    // 1) Ask Project to create a new model from this one
    std::pair<int, std::string>newIdName = m_project->duplicateMdn(index);
    int newIndex = newIdName.first;
    std::string newName = newIdName.second;

    // 2) Create a view for it
    std::pair<Mdn2d, Selection>* entry = m_project->at(newIndex);
    auto* w = new NumberDisplayWidget;
    w->setModel(&entry->first, &entry->second);
    w->setProject(m_project);

    // 3) Insert UI tab right after the source
    // int insertAt = index + 1;
    int newTab = m_tabWidget->insertTab(newIndex, w, QString::fromStdString(newName));
    m_tabWidget->setCurrentIndex(newTab);
    w->setFocus();
    if (m_ops) {
        m_ops->refreshTabNames();
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::copyTab(int index)
{
    Log_Debug3_H("index=" << index);
    if (!m_project) {
        Log_Debug3_T("No project");
        return;
    }
    m_project->copyMdn(index);
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::pasteTab(int insertAt)
{
    Log_Debug3_H("");
    if (!m_project) {
        Log_Debug3_T("");
        return;
    }
    m_project->pasteOnSelection(insertAt);
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::syncTabsToProject() {
    Log_Debug3_H("");
    if (!m_project) {
        Log_Debug3_T("No project");
        return;
    }
    const int n = m_tabWidget->count();
    for (int i = 0; i < n; ++i) {
        auto* view = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(i));
        if (!view) {
            continue;
        }

        std::pair<Mdn2d, Selection>* entry = m_project->at(i);
        Mdn2d& srcNum(entry->first);
        Selection& srcSel(entry->second);

        view->setModel(&srcNum, &srcSel);
        m_tabWidget->setTabText(i, QString::fromStdString(srcNum.name())); // add mdnName(int)
    }
    if (m_ops) {
        m_ops->refreshTabNames();
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::initFocusModel()
{
    Log_Debug3_H("initFocusModel");

    if (m_tabWidget) {
        m_tabWidget->setFocusPolicy(Qt::NoFocus);
        if (m_tabWidget->tabBar()) {
            m_tabWidget->tabBar()->setFocusPolicy(Qt::NoFocus);
        }
        connect(
            m_tabWidget,
            &QTabWidget::currentChanged,
            this,
            &mdn::gui::MainWindow::focusActiveGrid
        );
    }

    if (m_splitter) {
        m_splitter->setFocusPolicy(Qt::NoFocus);
    }

    if (menuBar()) {
        menuBar()->setFocusPolicy(Qt::StrongFocus);
    }

    if (qApp) {
        connect(qApp, &QApplication::focusChanged, this, &mdn::gui::MainWindow::onAppFocusChanged);
    }

    focusActiveGrid();
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::focusActiveGrid() {
    Log_Debug4_H("focusActiveGrid");

    QWidget* w = activeGridWidget();

    if (w) {
        w->setFocus(Qt::ShortcutFocusReason);
    }
    Log_Debug4_T("");
}


QWidget* mdn::gui::MainWindow::activeGridWidget() const
{
    QWidget* w{nullptr};

    if (m_tabWidget) {
        w = m_tabWidget->currentWidget();
    }

    return w;
}


bool mdn::gui::MainWindow::isFocusAllowedWidget(QWidget* w) const
{
    if (w == nullptr) {
        return false;
    }

    if (w->window() != this) {
        return true;
    }

    if (qobject_cast<QMenuBar*>(w) != nullptr) {
        return true;
    }

    if (qobject_cast<QMenu*>(w) != nullptr) {
        return true;
    }

    QWidget* grid = activeGridWidget();

    if (grid != nullptr) {
        if (w == grid) {
            return true;
        }
        if (grid->isAncestorOf(w)) {
            return true;
        }
    }

    if (m_command != nullptr) {
        if (w == m_command) {
            return true;
        }
        if (m_command->isAncestorOf(w)) {
            return true;
        }
    }

    return false;
}


void mdn::gui::MainWindow::onAppFocusChanged(QWidget* old, QWidget* now)
{
    Log_Debug4_H("onAppFocusChanged");

    if (!isFocusAllowedWidget(now)) {
        focusActiveGrid();
    }

    Log_Debug4_T("");
}


void mdn::gui::MainWindow::applySplitRatio() {
    QList<int> sizes = m_splitter->sizes();
    if (sizes.size() < 2) {
        return;
    }
    int total = 0;
    for (int s : sizes) {
        total += s;
    }
    if (total <= 0) {
        return;
    }
    int first = int(std::round(m_splitRatio * double(total)));
    int second = std::max(0, total - first);
    QList<int> newSizes;
    newSizes << first << second;
    m_splitter->setSizes(newSizes);
}
