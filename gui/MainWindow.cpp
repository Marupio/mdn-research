#include "MainWindow.hpp"

#include <QApplication>
#include <QEvent>
#include <QInputDialog>
#include <QLabel>
#include <QList>
#include <QMenuBar>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStatusBar>
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
    Log_Debug2_H("")
    createSplitter();
    createMenus();
    m_command = new CommandWidget(this);
    createStatusBar();
    setupLayout();
    setWindowTitle("MDN Editor");
    Log_Debug2_T("");
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
        Log_Debug4("removeTab(" << i << ")");
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
    Log_Debug_H("" << p);
    if (!m_project) {
        Log_Debug_T("");
        return;
    }

    Mdn2d& a = m_project->getMdn(p.indexA);
    Mdn2d& b = m_project->getMdn(p.indexB);

    if (p.dest == OpsController::Dest::InPlace) {
        switch (p.op) {
            case OpsController::Op::Add: {
                a += b;
                break;
            }
            case OpsController::Op::Subtract: {
                a -= b;
                break;
            }
            case OpsController::Op::Multiply: {
                a *= b;
                break;
            }
            case OpsController::Op::Divide: {
                a /= b;
                break;
            }
        }
        syncTabsToProject();
        m_tabWidget->setCurrentIndex(p.indexA);
        if (m_ops) {
            m_ops->refreshTabNames();
        }
        Log_Debug_T("");
        return;
    } else if (p.dest == OpsController::Dest::ToNew) {
        std::string requestedName = MdnQtInterface::fromQString(p.newName);
        if (requestedName.empty()) {
            requestedName = std::string("Result");
        }
        std::string suggestedName = m_project->suggestName(requestedName);
        Mdn2d ans(m_project->config(), suggestedName);
        Log_Debug2("suggestedName=" << suggestedName);
        switch (p.op) {
            case OpsController::Op::Add: {
                a.plus(b, ans);
                break;
            }
            case OpsController::Op::Subtract: {
                a.minus(b, ans);
                break;
            }
            case OpsController::Op::Multiply: {
                a.multiply(b, ans);
                break;
            }
            case OpsController::Op::Divide: {
                a.divide(b, ans);
                break;
            }
        }
        m_project->insertMdn(std::move(ans), p.indexA + 1);
        syncTabsToProject();
        m_tabWidget->setCurrentIndex(p.indexA);
        if (m_ops) {
            m_ops->refreshTabNames();
        }
        Log_Debug_T("");
        return;
    }
}


void mdn::gui::MainWindow::cycleEditMode() {
    // Shift-click cycles backward; normal click forward (optional UX sugar)
    const bool forward = !(QApplication::keyboardModifiers() & Qt::ShiftModifier);
    cycleGlobalEditMode(forward);
}


void mdn::gui::MainWindow::setGlobalEditMode(NumberDisplayWidget::EditMode m) {
    if (m_globalMode == m) {
        return;
    }
    m_globalMode = m;

    // Update all NDWs
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        if (auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(i)))
            ndw->setEditMode(m_globalMode);
    }

    // Update status button/label
    updateStatusModeText(m_globalMode);
}


void mdn::gui::MainWindow::toggleGlobalEditMode(NumberDisplayWidget::EditMode m) {
    if (m_globalMode != m) {
        setGlobalEditMode(m);
    } else {
        setGlobalEditMode(NumberDisplayWidget::EditMode::Overwrite);
    }
}


void mdn::gui::MainWindow::cycleGlobalEditMode(bool forward) {

    using M = NumberDisplayWidget::EditMode;
    M next;
    if (forward) {
        switch (m_globalMode) {
            case M::Overwrite:
                next = M::Add;
                break;
            case M::Add:
                next = M::Subtract;
                break;
            case M::Subtract:
                next = M::Overwrite;
                break;
        }
    } else {
        switch (m_globalMode) {
            case M::Overwrite:
                next = M::Subtract;
                break;
            case M::Add:
                next = M::Overwrite;
                break;
            case M::Subtract:
                next = M::Add;
                break;
        }
    }
    setGlobalEditMode(next);
}


void mdn::gui::MainWindow::onEditModeChanged(NumberDisplayWidget::EditMode m) {
    if (m != m_globalMode) {
        Log_Warn(
            "Unexpected edit mode change, to '" << NumberDisplayWidget::EditModeToString(m)
                << "', expecting '" << NumberDisplayWidget::EditModeToString(m_globalMode) << "'"
        );
    }
}


void mdn::gui::MainWindow::chooseModeOverwrite() {
    setGlobalEditMode(NumberDisplayWidget::EditMode::Overwrite);
}


void mdn::gui::MainWindow::chooseModeAdd() {
    setGlobalEditMode(NumberDisplayWidget::EditMode::Add);
}


void mdn::gui::MainWindow::chooseModeSubtract() {
    setGlobalEditMode(NumberDisplayWidget::EditMode::Subtract);
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
        Mdn2d& src = m_project->getMdn(index);
        Log_Debug4("Creating tab {'" << src.name() << "', " << index << "}");
        Selection& sel = m_project->getSelection(index);
        auto* ndw = new NumberDisplayWidget;
        ndw->setProject(m_project);
        ndw->setFocusPolicy(Qt::StrongFocus);
        ndw->setModel(&src, &sel);

        connect(ndw, &NumberDisplayWidget::statusCursorChanged, this,
            [this](int x, int y){
                if (m_statusCursor)
                    m_statusCursor->setText(QStringLiteral("(%1,%2)").arg(x).arg(y));
            });
        connect(
            ndw,
            &NumberDisplayWidget::statusSelectionChanged,
            this,
            &MainWindow::updateStatusSelectionText
        );
        connect(
            ndw,
            &NumberDisplayWidget::editModeChanged,
            this,
            &MainWindow::onEditModeChanged
        );
        connect(
            ndw,
            &NumberDisplayWidget::requestSetEditMode,
            this,
            &MainWindow::setGlobalEditMode
        );
        connect(
            ndw,
            &NumberDisplayWidget::requestToggleEditMode,
            this,
            &MainWindow::toggleGlobalEditMode
        );
        connect(
            ndw,
            &NumberDisplayWidget::requestCycleEditMode,
            this,
            &MainWindow::cycleGlobalEditMode
        );

        // Push current global mode into the new widget
        ndw->setEditMode(m_globalMode);

        std::string name = names[index];
        Log_Debug4("addTab(" << name << ")");
        QString qname = MdnQtInterface::toQString(name);
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
    updateStatusModeText(m_globalMode);
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::createTabForIndex(int index) {
    Log_Debug3_H("index=" << index);
    Mdn2d& entry = m_project->getMdn(index);
    Selection& sel = entry.selection();
    auto* view = new NumberDisplayWidget;
    view->setProject(m_project);
    view->setModel(&entry, &sel);

    const std::string& tabName(entry.name());
    QString tabNameQ = QString::fromStdString(tabName);
    Log_Info("insertTab(" << index << ", " << entry.name());
    int tab = m_tabWidget->insertTab(index, view, tabNameQ);
    m_tabWidget->setCurrentIndex(tab);
    view->setFocus();
    Log_Debug3_T("");
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


void mdn::gui::MainWindow::createStatusBar()
{
    auto* sb = statusBar();

    m_statusModeBtn = new QToolButton(this);
    m_statusModeBtn->setAutoRaise(true);
    m_statusModeBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_statusModeBtn->setText("OVER"); // initial
    connect(m_statusModeBtn, &QToolButton::clicked, this, &MainWindow::cycleEditMode);

    m_modeMenu = new QMenu(this);
    m_modeMenu->addAction("OVER", this, &MainWindow::chooseModeOverwrite);
    m_modeMenu->addAction("ADD +", this, &MainWindow::chooseModeAdd);
    m_modeMenu->addAction("SUB −", this, &MainWindow::chooseModeSubtract);
    m_statusModeBtn->setMenu(m_modeMenu);
    m_statusModeBtn->setPopupMode(QToolButton::MenuButtonPopup); // right side arrow; right-click opens menu

    sb->addPermanentWidget(m_statusModeBtn);

    m_statusMode   = new QLabel(this);
    m_statusCursor = new QLabel(this);
    m_statusSel    = new QLabel(this);

    m_statusMode->setText("OVER");
    m_statusCursor->setText("(0,0)");
    m_statusSel->setText("(empty)");

    // Left to right; make them compact
    sb->addPermanentWidget(m_statusMode,   0);
    sb->addPermanentWidget(new QLabel("  |  ", this), 0);
    sb->addPermanentWidget(m_statusCursor, 0);
    sb->addPermanentWidget(new QLabel("  |  ", this), 0);
    sb->addPermanentWidget(m_statusSel,    1); // stretch last one a bit
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

        Mdn2d& srcNum = m_project->getMdn(i);
        Selection& srcSel = srcNum.selection();

        view->setModel(&srcNum, &srcSel);
        m_tabWidget->setTabText(i, QString::fromStdString(srcNum.name())); // add mdnName(int)
    }
    if (m_ops) {
        m_ops->refreshTabNames();
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::updateStatusModeText(NumberDisplayWidget::EditMode m) {
    if (!m_statusMode) {
        return;
    }
    switch (m) {
        case NumberDisplayWidget::EditMode::Overwrite: {
            m_statusModeBtn->setText("OVER");
            break;
        }
        case NumberDisplayWidget::EditMode::Add: {
            m_statusModeBtn->setText("ADD +");
            break;
        }
        case NumberDisplayWidget::EditMode::Subtract: {
            m_statusModeBtn->setText("SUB −");
            break;
        }
    }
}


void mdn::gui::MainWindow::updateStatusSelectionText(const mdn::Selection& s) {
    if (!m_statusSel) {
        return;
    }
    if (!s.rect().isValid()) {
        m_statusSel->setText(QStringLiteral("(empty)"));
        return;
    }
    const Rect& r = s.rect();
    if (r.isSingleCoord()) {
        m_statusSel->setText(
            QStringLiteral("(1×1)")
        );
    } else {
        const Coord& rMin = r.min();
        const Coord& rMax = r.max();
        const Coord& c1 = s.cursor1();
        const int w = r.width();
        const int h = r.height();
        m_statusSel->setText(
            QStringLiteral("[%1,%2]→[%3,%4] (%5×%6)")
                .arg(rMin.x()).arg(rMin.y())
                .arg(rMax.x()).arg(rMax.y())
                .arg(w).arg(h)
        );
    }
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
