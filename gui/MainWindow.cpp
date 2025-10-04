#include "MainWindow.hpp"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyle>
#include <QTabBar>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>

#include "../library/Logger.hpp"
#include "../library/Carryover.hpp"

#include "GuiTools.hpp"
#include "HelpDialog.hpp"
#include "HoverPeekTabWidget.hpp"
#include "MarkerWidget.hpp"
#include "MdnQtInterface.hpp"
#include "NumberDisplayWidget.hpp"
#include "OpsController.hpp"
#include "ProjectPropertiesDialog.hpp"
#include "StatusDisplayWidget.hpp"

int mdn::gui::MainWindow::nStartMdnDefault = 3;
bool mdn::gui::MainWindow::enableCommandWidget = false;


mdn::gui::MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_project(nullptr)
{
    Log_Debug2_H("")
    createSplitter();
    createMenus();
    createCommandWidget();
    createStatusBar();
    setupLayout();
    setWindowTitle("MDN Editor");
    Log_Debug2_T("");
}


mdn::gui::MainWindow::MainWindow(QWidget *parent, Mdn2dConfig* cfg) :
    QMainWindow(parent),
    m_project(nullptr)
{
    Log_Debug2_H("")
    createSplitter();
    createMenus();
    createCommandWidget();
    createStatusBar();
    setupLayout(cfg);
    // absorbProjectProperties(dlg);
    // setWindowTitle("MDN Editor");
    Log_Debug2_T("");
}


mdn::gui::MainWindow::~MainWindow()
{
    // 1) Guard against app-wide signals firing during teardown.
    if (qApp) {
        disconnect(
            qApp,
            &QApplication::focusChanged,
            this,
            &mdn::gui::MainWindow::onAppFocusChanged
        );
    }

    // 2) Stop splitter -> MainWindow traffic, and remove event filter.
    if (m_splitter) {
        m_splitter->removeEventFilter(this);
        disconnect(
            m_splitter,
            &QSplitter::splitterMoved,
            this,
            &mdn::gui::MainWindow::onSplitterMoved
        );
    }

    // 3) Project is a QObject child of MainWindow (Project(this)), disconnect here for clarity.
    if (m_project) {
        disconnect(m_project, nullptr, this, nullptr);
    }

    // 4) Being extra cautious: nuke OpsController hookups.
    if (m_ops) {
        disconnect(m_ops, nullptr, this, nullptr);
    }
}


bool mdn::gui::MainWindow::setCommandVisibility(bool visible) {
    if (!m_command) {
        return false;
    }
    if (visible == m_command->isVisible()) {
        return true;
    }
    m_command->setVisible(visible);
    weldSliderToBottom(!visible);
    if (visible && m_splitter) {
        QList<int> sizes = m_splitter->sizes();
        sizes[1] = std::max(200, sizes[1]);
        m_splitter->setSizes(sizes);
    }
    return true;
}


bool mdn::gui::MainWindow::commandVisibility() const {
    if (!m_command) {
        return false;
    }
    return m_command->isVisible();
}


bool mdn::gui::MainWindow::clearStatus() {
    Log_Debug_H("");
    if (m_status) {
        m_status->clearMessage();
        Log_Debug_T("true");
        return true;
    }
    Log_Debug_T("false");
    return false;
}


bool mdn::gui::MainWindow::showStatus(QString message, int timeOut, bool forceUpdate) {
    Log_Debug_H("message=[" << message.toStdString() << "]");
    if (m_status) {
        m_status->showMessage(message, timeOut);

        if (forceUpdate) {
            // Hacky - make sure message shows up
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }

        Log_Debug_T("");
        return true;
    }
    Log_Debug_T("");
    return true;
    // QStatusBar* status = statusBar();
    // if (!status) {
    //     Log_Debug_T("No status");
    //     return false;
    // }
    // status->showMessage(message, timeOut);
    // Log_Debug_T("");
    // return true;
}


bool mdn::gui::MainWindow::onCancelRequested(NumberDisplayWidget* echoBack) {
    if (m_ops) {
        if (m_ops->cancelRequested()) {
            // Absorbed the cancel, no echo back
            return true;
        }
    }
    // No action taken, check echo back
    if (echoBack) {
        echoBack->cancelEchoBack();
        return true;
    }
    // No one used the cancel
    return false;
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
    menu.addSeparator();
    QAction* actSaveAs    = menu.addAction("Save as...");
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
        // insert after current tab
        pasteTab(index + 1);
    } else if (picked == actSaveAs) {
        saveMdn2d(index);
    } else if (picked == actDelete) {
        closeTab(index);
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

    // No need to account for 'plusTab' as cast will fail
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

    Log_Debug3("Removing plusTab");
    removePlusTab();
    QSignalBlocker blockTabs(m_tabWidget);

    const int n = m_tabWidget->count();
    for (int i = n - 1; i >= 0; --i) {
        QWidget* w = m_tabWidget->widget(i);
        Log_Debug4("removeTab(" << i << ")");
        m_tabWidget->removeTab(i);
        delete w;
    }

    // This will create the plusTab as well
    createTabs();
    ensureTabCorner();

    int idx = currentIndex;
    const int count = m_tabWidget->count();
    if (idx < 0) {
        idx = 0;
    } else {
        if (idx >= count) {
            idx = count - 1;
        }
    }
    // Accounting for 'plusTab'
    if (count > 1) {
        setActiveTab(idx);
    }

    if (centralWidget()) {
        centralWidget()->setUpdatesEnabled(true);
    }

    focusActiveGrid();

    Log_Debug2_T("");
}


void mdn::gui::MainWindow::onProjectProperties() {
    doProjectProperties();
}


void mdn::gui::MainWindow::onTabPeek(int idx)
{
    Log_Debug3_H("idx=" << idx);
    if (!m_tabWidget) {
        Log_Debug3_T("No tabWidget");
        return;
    }

    if (hasPlusTab() && idx == m_tabWidget->count() - 1) {
        Log_Debug3_T("Cannot peek plusTab");
    }

    if (!m_peekActive) {
        // First time entering peek: remember explicit selection
        m_explicitIndex = m_tabWidget->currentIndex();
        m_peekActive = true;
        if (m_explicitIndex >= 0)
            GuiTools::setTabPeekHighlight(m_tabWidget, m_explicitIndex, false);
    }

    // Switch to hovered tab
    if (idx != m_tabWidget->currentIndex()) {
        // Clear old peek highlight (if any)
        const int prev = m_tabWidget->currentIndex();
        GuiTools::setTabPeekHighlight(m_tabWidget, prev, false);

        m_tabWidget->setCurrentIndex(idx);

        // Apply peek highlight to the hovered/selected tab
        GuiTools::setTabPeekHighlight(m_tabWidget, idx, true);
    }

    // Cancel any pending restore; we’re still peeking
    if (m_peekRestore.isActive()) m_peekRestore.stop();
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::onTabPeekEnd()
{
    Log_Debug3_H("");
    if (!m_tabWidget || !m_peekActive) {
        Log_Debug3_T("No tabWidget or no activePeek");
        return;
    }

    // If user didn’t commit, restore the explicit tab
    const int curr = m_tabWidget->currentIndex();
    GuiTools::setTabPeekHighlight(m_tabWidget, curr, false);

    if (m_explicitIndex >= 0 && m_explicitIndex != curr) {
        m_tabWidget->setCurrentIndex(m_explicitIndex);
    }
    m_peekActive = false;
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::onTabCommit(int idx) {
    Log_Debug3_H("idx=" << idx);
    if (!m_tabWidget) {
        Log_Debug3_T("No tabWidget");
        return;
    }

    const int n = m_tabWidget->tabBar()->count();
    if (idx == n - 1) {
        // It's the plus tab (add new tab)
        Log_Debug3("user clicked '+' tab");
        bool result = onNewMdn2d();
        Log_Debug3_T("result=" << result);
        return;
    }

    // Clicking a tab “locks” it in; this also ends any peek
    m_explicitIndex = idx;
    // Clear peek highlight from any tab
    for (int i = 0; i < n; ++i) {
        GuiTools::setTabPeekHighlight(m_tabWidget, i, false);
    }

    m_peekActive = false;
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::onChangedActiveIndex(int idx) {
    if (m_project) {
        m_project->setActiveMdn(idx);
    }
}


bool mdn::gui::MainWindow::onNewProject() {
    Log_Debug2_H("");
    bool result = createNewProject();
    Log_Debug2_T("");
    return result;
}


bool mdn::gui::MainWindow::onNewMdn2d() {
    Log_Debug3_H("");
    int idx = -1;
    if (m_project) {
        idx = m_project->activeIndex()+1;
    }
    bool result = onNewNamedMdn2d("", idx);
    Log_Debug3_T("result=" << result);
    return result;
}


bool mdn::gui::MainWindow::onNewNamedMdn2d(QString name, int index) {
    Log_Debug2_H("name=" << name.toStdString() << ",index=" << index);
    // To do - get user input for name
    bool result = createNewMdn2d(name, index, true);
    Log_Debug2_T("result = " << result);
    return result;
}


bool mdn::gui::MainWindow::onSaveProject() {
    Log_Debug2_H("");
    if (m_globalConfig.parentPath().empty()) {
        Log_Debug3("Not previously saved, reverting to onSaveProjectAs...");
        return onSaveProjectAs();
    }

    const QString name = QString::fromStdString(m_project->name());
    const QString basePath = QString::fromStdString(m_project->path());

    // Joins with exactly one separator and normalizes "." / ".."
    const QString fullPath = QDir(basePath).filePath(name);
    const QString fullPathNative = QDir::toNativeSeparators(fullPath);
    bool result = saveProjectToPath(fullPathNative);
    Log_Debug2_T("result=" << result);
    return result;
}


bool mdn::gui::MainWindow::onSaveProjectAs() {
    Log_Debug2_H("");
    if (!m_project) {
        Log_WarnQ("No project open, cannot save");
        Log_Debug2_T("");
        return false;
    }
    // Suggest default path in Documents with project name
    QString suggestedPath = QString::fromStdString(m_project->path());
    if (suggestedPath.size() == 0) {
        QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        suggestedPath = docs + "/";
    } else {
        suggestedPath += "/";
    }
    QString suggested = suggestedPath + QString::fromStdString(m_project->name()) + ".mdnproj";

    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Save Project"),
        suggested,
        tr("MDN Project (*.mdnproj);;All Files (*)")
    );

    if (path.isEmpty()) {
        Log_Debug2_T("user cancelled")
        showStatus(tr("Cancelled save"), 2000);
        return false;
    }

    bool result = saveProjectToPath(path);
    Log_Debug2_T("result=" << result);
    return result;
}


bool mdn::gui::MainWindow::saveProjectToPath(const QString& path) {
    bool ok = m_project->saveToFile(path.toStdString());
    if (!ok) {
        QMessageBox::warning(
            this,
            tr("Save Project"),
            tr("Failed to save project.")
        );
        Log_Debug2_T("failed")
        return false;
    }

    // Convert to folder only
    QFileInfo info(path);
    QString folder = info.absolutePath();
    QString baseName = info.baseName();

    // Pass to project
    std::string parentPath = folder.toStdString();
    std::string parentName = baseName.toStdString();
    m_project->setPath(parentPath);
    m_project->setName(parentName);
    m_globalConfig.setParentPath(parentPath);
    m_globalConfig.setParentName(parentPath);

    setWindowTitle(baseName);
    showStatus(tr("Project saved"), 2000);
    Log_Debug2_T("ok")
    return true;
}


bool mdn::gui::MainWindow::onOpenProject() {
    Log_Debug2_H("");
    bool result = openProject(true);
    Log_Debug2_T("result=" << result);
    return result;
}


bool mdn::gui::MainWindow::onSaveMdn2d() {
    Log_Debug2_H("");
    bool result = saveMdn2d(-1);
    Log_Debug2_T("result=" << result);
    return result;
}


bool mdn::gui::MainWindow::saveMdn2d(int idx) {
    Log_Debug2_H("");

    if (!m_project) {
        Log_WarnQ("No project open, cannot save Mdn2d");
        Log_Debug2_T("no project");
        return false;
    }

    if (idx < 0) {
        idx = m_tabWidget ? m_tabWidget->currentIndex() : m_project->activeIndex();
    }
    if (idx < 0) {
        Log_WarnQ("No active tab to save");
        Log_Debug2_T("no active");
        return false;
    }

    Mdn2d* src = m_project->getMdn(idx);
    if (!src) {
        Log_WarnQ("Failed to acquire Mdn for tab " << idx << ", cannot save");
        Log_Debug2_T("");
        return false;
    }
    const QString baseName = QString::fromStdString(src->name());

    QString suggestedDir = QString::fromStdString(m_project->path());
    if (suggestedDir.isEmpty()) {
        suggestedDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    const QString filters =
        tr("Binary (*.mdnbin);;"
           "Pretty text (*.mdntxt);;"
           "Utility csv text (*.mdncsv);;"
           "Utility tsv text (*.mdntsv);;"
           "All Files (*)");

    QString selectedFilter;
    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Save Number"),
        suggestedDir + "/" + baseName + ".mdnbin",
        filters,
        &selectedFilter
    );

    if (path.isEmpty()) {
        Log_Debug2_T("cancelled");
        showStatus(tr("Cancelled save"), 2000);
        return false;
    }

    QFileInfo info(path);
    QString ext = info.suffix().toLower();
    if (selectedFilter.contains("Binary") && ext != "mdnbin") {
        path += ".mdnbin";
    } else if (selectedFilter.contains("Pretty") && ext != "mdntxt") {
        path += ".mdntxt";
    } else if (selectedFilter.contains("Utility csv") && ext != "mdncsv") {
        path += ".mdncsv";
    } else if (selectedFilter.contains("Utility tsv") && ext != "mdntsv") {
        path += ".mdntsv";
    }

    std::ofstream ofs(path.toStdString(), std::ios::binary);
    if (!ofs.is_open()) {
        QMessageBox::warning(this, tr("Save Number"), tr("Cannot open file for writing."));
        Log_Debug2_T("open fail");
        return false;
    }

    if (selectedFilter.contains("Binary")) {
        src->saveBinary(ofs);
    } else if (selectedFilter.contains("Pretty")) {
        src->saveTextPretty(ofs, true, true);
    } else if (selectedFilter.contains("Utility csv")) {
        src->saveTextUtility(ofs, CommaTabSpace::Comma);
    } else if (selectedFilter.contains("Utility tsv")) {
        src->saveTextUtility(ofs, CommaTabSpace::Tab);
    } else {
        if (ext == "mdnbin") {
            src->saveBinary(ofs);
        } else if (ext == "mdntxt") {
            src->saveTextPretty(ofs, true, true);
        } else if (ext == "mdncsv") {
            src->saveTextUtility(ofs, CommaTabSpace::Comma);
        } else if (ext == "mdntsv") {
            src->saveTextUtility(ofs, CommaTabSpace::Tab);
        } else {
            src->saveBinary(ofs);
        }
    }

    const bool ok = ofs.good();
    if (!ok) {
        QMessageBox::warning(this, tr("Save Number"), tr("Failed to write file."));
        Log_Debug2_T("write fail");
        return false;
    }

    showStatus(tr("Tab(%1) saved").arg(idx), 2000);
    // showStatus(tr("Number saved"), 2000);
    Log_Debug2_T("ok");
    return true;
}


bool mdn::gui::MainWindow::onOpenMdn2d() {
    Log_Debug2_H("");

    if (!m_project) {
        Log_WarnQ("No project open, cannot load Mdn2d");
        Log_Debug2_T("no project");
        return false;
    }

    QString startDir = QString::fromStdString(m_project->path());
    if (startDir.isEmpty()) {
        startDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    const QString filters =
        tr("Binary (*.mdnbin);;"
           "Pretty text (*.mdntxt);;"
           "Utility csv text (*.mdncsv);;"
           "Utility tsv text (*.mdntsv);;"
           "All Files (*)");

    QString path = QFileDialog::getOpenFileName(
        this,
        tr("Open Number"),
        startDir,
        filters
    );

    if (path.isEmpty()) {
        Log_Debug2_T("cancelled");
        return false;
    }

    std::ifstream ifs(path.toStdString(), std::ios::binary);
    if (!ifs.is_open()) {
        QMessageBox::warning(this, tr("Open Number"), tr("Cannot open file for reading."));
        Log_Debug2_T("open fail");
        return false;
    }

    Mdn2d num = Mdn2d::NewInstance(m_globalConfig, "");
    const QString ext = QFileInfo(path).suffix().toLower();

    Log_Debug3("Checking extension: [" << ext.toStdString() << "]");
    try {
        if (ext == "mdnbin") {
            Log_Debug4_H("loadBinary dispatch");
            num.loadBinary(ifs);
            Log_Debug4_T("loadBinary return");
        } else {
            Log_Debug4_H("loadText dispatch");
            num.loadText(ifs);
            Log_Debug4_T("loadText return");
        }
    } catch (const std::exception& e) {
        Log_Debug4_T("caught an error");
        QMessageBox::warning(this, tr("Open Number"), tr("Failed to load number."));
        Log_Error(e.what());
        Log_Debug2_T("load fail");
        return false;
    }

    // num.setConfig(m_globalConfig);
    // appends to end by convention
    Log_Debug4("importing number");
    if (!m_project->importMdn(std::move(num), -1)) {
        // Incompatible number
        Log_WarnQ(""
            << "Incompatible import\n"
            << "Attempting to load a Mdn2d with config:\n"
            << num.config() << "\n"
            << "when the project config is:\n"
            << m_globalConfig << "\n"
            << "Dissimilar bases means this number is incompatible. Change your project's base "
            << "to " << num.config().base() << " and try again."
        );
        Log_Debug2_T("number incompatible");
        return false;
    }
    // rebuild tabs; preserves current selection logic
    syncTabsToProject();
    showStatus(tr("Loaded new tab"), 2000);
    Log_Debug2_T("ok");
    return true;
}


bool mdn::gui::MainWindow::onZeroMdn2d() {
    if (!m_project) {
        showStatus(tr("No project, cannot zero tabs"), 2000);
        return false;
    }
    const int idx = m_project->activeIndex();
    Mdn2d* tgt(m_project->getMdn(idx));
    if (!tgt) {
        showStatus(tr("Failed to reach data for tab %1").arg(idx), 2000);
        return false;
    }
    const std::string& tgtName(tgt->name());
    const Rect& bounds(tgt->bounds());
    const QString tabName = QString::fromStdString(tgt->name());
    if (bounds.empty()) {
        showStatus(
            tr("No non-zero digits exist for '%1'")
                .arg(MdnQtInterface::toQString(tgtName)
            ),
            2000
        );
        if (auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(idx))) {
            ndw->armCentreViewOnOrigin();
            ndw->update();
        }
    }
    std::ostringstream oss;
    oss << "Zero all digits on " << tgtName << "?  This cannot be undone.\n\n"
        << "Are you sure?";
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("Zero all digits?"),
        tr(oss.str().c_str()),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No // Default button
    );
    if (reply != QMessageBox::Yes) {
        Log_Debug_T("User rejected zero operation");
        clearStatus();
        showStatus(tr("Zeroing cancelled"), 2000);
        return false;
    }
    // User said Yes

    tgt->setToZero(bounds);
    if (auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(idx))) {
        ndw->armCentreViewOnOrigin();
        ndw->update();
        showStatus(
            tr("Tab '%1' zeroed")
                .arg(MdnQtInterface::toQString(tgtName)
            ),
            2000
        );
    }
    return true;
}


bool mdn::gui::MainWindow::onCloseProject() {
    Log_Debug2_H("");
    bool result = confirmedCloseProject(true);
    Log_Debug2_T("result=" << result);
    return result;
}


void mdn::gui::MainWindow::onSelectAll() {
    if (!m_tabWidget) {
        showStatus(tr("Select all >> nothing to select"), 2000);
        return;
    }
    int index = m_tabWidget->currentIndex();
    if (auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(index))) {
        ndw->selectAllBounds();
    }
}


void mdn::gui::MainWindow::onRequestFitBottomToContents() {
    fitBottomToContents();
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


void mdn::gui::MainWindow::onOpsPlan(const OperationPlan& p) {
    Log_Debug_H("" << p);
    if (!m_project) {
        Log_Debug_T("");
        return;
    }

    Mdn2d* aPtr = m_project->getMdn(p.indexA);
    Mdn2d* bPtr = m_project->getMdn(p.indexB);
    if (!aPtr || !bPtr) {
        Log_WarnQ(
            "Failed to acquire reference to Mdn2d operand, either "
                << p.indexA << " or " << p.indexB << ", cannot continue.";
        );
        return;
    }
    showStatus(tr("Calculating . . . (please wait, app will hang until complete)"), 0, true);
    Mdn2d& a = *aPtr;
    Mdn2d& b = *bPtr;
    if (p.op == Operation::Divide) {
        divide(p, a, b);
        Log_Debug_T("");
        return;
    }
    if (p.indexDest >= 0) {
        const int t = p.indexDest;
        if (t == p.indexA) {
            switch (p.op) {
                case Operation::Add: {
                    Log_Debug4_H("Dispatch a += b");
                    a += b;
                    Log_Debug4_T("a += b return");
                    break;
                }
                case Operation::Subtract: {
                    Log_Debug4_H("Dispatch a -= b");
                    a -= b;
                    Log_Debug4_T("a -= b return");
                    break;
                }
                case Operation::Multiply: {
                    Log_Debug4_H("Dispatch a *= b");
                    a *= b;
                    Log_Debug4_T("a *= b return");
                    break;
                }
                // case Operation::Divide: {
                //     Log_Debug4_H("Dispatch a /= b");
                //     a /= b;
                //     Log_Debug4_T("a /= b return");
                //     break;
                // }
            }
            syncTabsToProject();
            setActiveTab(p.indexA);
            clearStatus();
            showStatus(tr("Calculation complete"), 2000);
            Log_Debug_T("");
            return;
        } else {
            Mdn2d* ansPtr = m_project->getMdn(t);
            if (ansPtr->data_index().size()) {
                // TODO use peek feature to show the number in question
                std::ostringstream oss;
                oss << "If you proceed, the answer will overwrite "
                    << "tab '" << ansPtr->name() << "'.\n\n"
                    << "Are you sure?";
                QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    "Overwrite Number?",
                    tr(oss.str().c_str()),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No // Default button
                );
                if (reply != QMessageBox::Yes) {
                    Log_Debug_T("User rejected overwrite");
                    clearStatus();
                    showStatus(tr("Calculation cancelled"), 2000);
                    return;
                }
                // User said Yes
            }
            ansPtr->clear();
            Mdn2d& ans = *ansPtr;
            switch (p.op) {
                case Operation::Add:{
                    Log_Debug4_H("Dispatch a.plus(b, ans);");
                    a.plus(b, ans);
                    Log_Debug4_T("Return a.plus(b, ans);");
                    break;
                }
                case Operation::Subtract:{
                    Log_Debug4_H("Dispatch a.minus(b, ans);");
                    a.minus(b, ans);
                    Log_Debug4_T("Return a.minus(b, ans);");
                    break;
                }
                case Operation::Multiply:{
                    Log_Debug4_H("Dispatch a.multiply(b, ans);");
                    a.multiply(b, ans);
                    Log_Debug4_T("Return a.multiply(b, ans);");
                    break;
                }
                // case Operation::Divide:{
                //     Log_Debug4_H("Dispatch a.divide(b, ans);");
                //     a.divide(b, ans);
                //     Log_Debug4_T("Return a.divide(b, ans);");
                //     break;
                // }
            }
            syncTabsToProject();
            setActiveTab(t);
            clearStatus();
            showStatus(tr("Calculation complete"), 2000);
            Log_Debug_T("");
            return;
        }
    } else {
        // p.indexDest < 0 we are writing answer to a new tab
        std::string requestedName = MdnQtInterface::fromQString(p.newName);
        if (requestedName.empty()) {
            requestedName = std::string("Result");
        }
        std::string suggestedName = m_project->suggestName(requestedName);
        Mdn2d ans(m_globalConfig, suggestedName);
        Log_Debug2("suggestedName=" << suggestedName);
        switch (p.op) {
            case Operation::Add: {
                Log_Debug4("Dispatch a.plus(b, ans)");
                a.plus(b, ans);
                Log_Debug4("a.plus(b, ans) return");
                break;
            }
            case Operation::Subtract: {
                Log_Debug4("Dispatch a.minus(b, ans)");
                a.minus(b, ans);
                Log_Debug4("a.minus(b, ans) return");
                break;
            }
            case Operation::Multiply: {
                Log_Debug4("Dispatch a.multiply(b, ans)");
                a.multiply(b, ans);
                Log_Debug4("a.multiply(b, ans) return");
                break;
            }
            // case Operation::Divide: {
            //     Log_Debug4("Dispatch a.divide(b, ans)");
            //     a.divide(b, ans);
            //     Log_Debug4("a.divide(b, ans) return");
            //     break;
            // }
        }
        m_project->appendMdn(std::move(ans));
        syncTabsToProject();
        setActiveTab(m_project->size()-1);
        clearStatus();
        showStatus(tr("Calculation complete"), 2000);
        Log_Debug_T("");
        return;
    }
}


void mdn::gui::MainWindow::onDivisionIterateRequested() {
    Log_Debug3_H("");
    if (!m_ad_operandA) {
        Log_Warn("Out-of-sequence division iteration request");
        Log_Debug3_T("");
        return;
    }
    static_cast<void>(
        m_ad_operandA->divideIterate(
            10,
            *m_ad_operandB,
            *m_ad_destination,
            *m_ad_remainder,
            m_ad_remMag,
            m_globalConfig.fraxis()
        )
    );
    Log_Debug3("m_ad_remMag=" << m_ad_remMag);
    if (m_ad_remMag == 0) {
        onDivisionStopRequested();
    }
    if (m_project) {
        int activeIndex = m_project->activeIndex();
        if (auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(activeIndex))) {
            ndw->update();
        }
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::onDivisionStopRequested() {
    m_ad_operandA = nullptr;
    m_ad_operandB = nullptr;
    m_ad_remainder = nullptr;
    m_ad_destination = nullptr;
    m_ad_remMag = -1.0;
}


void mdn::gui::MainWindow::onTransposeClicked() {
    if (m_project) {
        Mdn2d* tgt(m_project->activeMdn());
        if (!tgt) {
            showStatus(tr("No active MDN tab"), 2000);
            return;
        }
        tgt->transpose();
        showStatus(tr("Transpose complete >> x ←→ y"), 2000);
        int activeIndex = m_project->activeIndex();
        if (auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(activeIndex))) {
            ndw->update();
        }

    }
}


void mdn::gui::MainWindow::onCarryOverClicked() {
    if (m_project) {
        Mdn2d* tgt(m_project->activeMdn());
        if (!tgt) {
            return;
        }
        const Selection& sel = tgt->selection();
        const Coord& c1 = sel.cursor1();
        Carryover co = tgt->checkCarryover(c1);
        if (co == Carryover::Invalid) {
            showStatus(tr("Invalid carryover"), 2000);
        } else {
            tgt->carryover(c1);
            int activeIndex = m_project->activeIndex();
            if (auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(activeIndex))) {
                ndw->update();
            }
        }
    }
}


void mdn::gui::MainWindow::onCarryPosClicked() {
    if (m_project) {
        Mdn2d* tgt(m_project->activeMdn());
        if (!tgt) {
            showStatus(tr("No active MDN tab"), 2000);
            return;
        }
        const Selection& sel = tgt->selection();
        const Coord& c1 = sel.cursor1();
        CoordSet changed = tgt->carryoverCleanupAll(SignConvention::Positive);
        int activeIndex = m_project->activeIndex();
        if (auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(activeIndex))) {
            ndw->update();
        }
        showStatus(tr("Carryover(+) changed %1 digits").arg(changed.size()), 2000);
    }
}


void mdn::gui::MainWindow::onCarryNegClicked() {
    if (m_project) {
        Mdn2d* tgt(m_project->activeMdn());
        if (!tgt) {
            showStatus(tr("No active MDN tab"), 2000);
            return;
        }
        const Selection& sel = tgt->selection();
        const Coord& c1 = sel.cursor1();
        CoordSet changed = tgt->carryoverCleanupAll(SignConvention::Negative);
        int activeIndex = m_project->activeIndex();
        if (auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(activeIndex))) {
            ndw->update();
        }
        showStatus(tr("Carryover(-) changed %1 digits").arg(changed.size()), 2000);
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


void mdn::gui::MainWindow::setGlobalFontSize(int pt) {
    if (m_globalFontSize == pt) {
        return;
    }
    m_globalFontSize = pt;

    // Update all NDWs
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        if (auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(i)))
            ndw->setFontPointSize(m_globalFontSize);
    }
    if (m_status) {
        m_status->setFontSize(m_globalFontSize);
        showStatus(tr("New font size %1").arg(m_globalFontSize), 2000);
    }
}


void mdn::gui::MainWindow::setGlobalConfig(Mdn2dConfig c, bool force) {
    Log_Debug2_H("newConfig=" << c << ", currentConfig=" << m_globalConfig);

    If_Log_Showing_Debug3(
        Log_Debug3("parentName,was:" << m_globalConfig.parentName() << ",is:" << c.parentName());
        Log_Debug3("parentPath,was:" << m_globalConfig.parentPath() << ",is:" << c.parentPath());
    );
    bool idChanged = false;
    if (
        m_globalConfig.parentName() != c.parentName() ||
        m_globalConfig.parentPath() != c.parentPath()
    ) {
        Log_Debug2("Identity has changed");
        m_globalConfig.setParentName(c.parentName());
        m_globalConfig.setParentPath(c.parentPath());
        idChanged = true;
    }

    if (!force && !idChanged && m_globalConfig == c) {
        Log_Debug2_T("no changes");
        return;
    }
    m_globalConfig = c;
    if (m_project) {
        m_globalConfig.setParent(*m_project);
        m_project->setConfig(m_globalConfig);
    }
    updateStatusFraxisText(c.fraxis());
    updateStatusSignConventionText(c.signConvention());
    Log_Debug2_T("");
}


void mdn::gui::MainWindow::updateGlobalConfig(Mdn2dConfig c, bool force) {
    Log_Debug2_H("newConfig=" << c << ", currentConfig=" << m_globalConfig);
    if (!force && m_globalConfig == c) {
        Log_Debug2_T("no changes");
        return;
    }
    m_globalConfig = c;
    if (m_project) {
        m_globalConfig.setParent(*m_project);
        m_project->setConfig(m_globalConfig);
    }
    updateStatusFraxisText(c.fraxis());
    updateStatusSignConventionText(c.signConvention());
    Log_Debug2_T("");
}


void mdn::gui::MainWindow::cycleFraxis() {
    if (!m_project) {
        return;
    }

    Mdn2dConfig cfg = m_project->config();
    const Fraxis next = (cfg.fraxis() == Fraxis::X) ? Fraxis::Y : Fraxis::X;
    cfg.setFraxis(next);

    // Route through Project::setConfig so impact prompts/updates propagate to all tabs
    // shows impact dialogs if needed
    updateGlobalConfig(cfg);
}


void mdn::gui::MainWindow::chooseFraxisX() {
    if (!m_project) return;
    Mdn2dConfig cfg = m_project->config();
    if (cfg.fraxis() == Fraxis::X) return;
    showStatus(tr("Fraxis >> X"), 2000);
    cfg.setFraxis(Fraxis::X);
    updateGlobalConfig(cfg);
}


void mdn::gui::MainWindow::chooseFraxisY() {
    if (!m_project) return;
    Mdn2dConfig cfg = m_project->config();
    if (cfg.fraxis() == Fraxis::Y) return;
    showStatus(tr("Fraxis >> Y"), 2000);
    cfg.setFraxis(Fraxis::Y);
    updateGlobalConfig(cfg);
}


void mdn::gui::MainWindow::cycleSignConvention() {
    if (!m_project) {
        return;
    }
    Mdn2dConfig cfg = m_project->config();
    SignConvention next;
    switch (cfg.signConvention()) {
        case SignConvention::Positive:
            next = SignConvention::Neutral;
            break;
        case SignConvention::Neutral:
            next = SignConvention::Negative;
            break;
        case SignConvention::Negative:
            next = SignConvention::Positive;
            break;
    }
    cfg.setSignConvention(next);

    // Route through Project::setConfig so impact prompts/updates propagate to all tabs
    // shows impact dialogs if needed
    updateGlobalConfig(cfg);
}


void mdn::gui::MainWindow::chooseSignConventionPositive() {
    if (!m_project) return;
    Mdn2dConfig cfg = m_project->config();
    if (cfg.signConvention() == SignConvention::Positive) return;
    showStatus(tr("SignConvention >> Positive"), 2000);
    cfg.setSignConvention(SignConvention::Positive);
    updateGlobalConfig(cfg);
}


void mdn::gui::MainWindow::chooseSignConventionNeutral() {
    if (!m_project) return;
    Mdn2dConfig cfg = m_project->config();
    if (cfg.signConvention() == SignConvention::Neutral) return;
    showStatus(tr("SignConvention >> Neutral"), 2000);
    cfg.setSignConvention(SignConvention::Neutral);
    updateGlobalConfig(cfg);
}


void mdn::gui::MainWindow::chooseSignConventionNegative() {
    if (!m_project) return;
    Mdn2dConfig cfg = m_project->config();
    if (cfg.signConvention() == SignConvention::Negative) return;
    showStatus(tr("SignConvention >> Negative"), 2000);
    cfg.setSignConvention(SignConvention::Negative);
    updateGlobalConfig(cfg);
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
    int maxIndex = count - 1 - (m_plusTab ? 1 : 0);
    if (idx < maxIndex) {
        int activeIndex = idx + 1;
        m_tabWidget->setCurrentSelectedIndex(idx + 1);
        focusActiveGrid();
        if (m_project) {
            m_project->setActiveMdn(activeIndex);
        }
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
        int activeIndex = idx - 1;
        m_tabWidget->setCurrentSelectedIndex(activeIndex);
        focusActiveGrid();
        if (m_project) {
            m_project->setActiveMdn(activeIndex);
        }
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
    // Cannot move last tab rightward over plusTab, maxTab is count-1-plusTab - 1
    const int maxIndex = hasPlusTab() ? count - 3 : count - 2;
    if (idx >= 0 && idx < maxIndex) {
        QTabBar* bar = m_tabWidget->tabBar();
        int activeIndex = idx + 1;
        bar->moveTab(idx, activeIndex);
        m_tabWidget->setCurrentSelectedIndex(activeIndex);
        focusActiveGrid();
        if (m_project) {
            m_project->setActiveMdn(activeIndex);
        }
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
        int activeIndex = idx - 1;
        bar->moveTab(idx, activeIndex);
        m_tabWidget->setCurrentSelectedIndex(activeIndex);
        focusActiveGrid();
        if (m_project) {
            m_project->setActiveMdn(activeIndex);
        }
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::slotDebugShowAllTabs() {
    Log_Debug3_H("");
    if (m_project)
    {
        std::ostringstream oss;
        m_project->debugShowAllTabs(oss);
        const Mdn2dConfig& c = m_project->config();
        oss << std::endl;
        oss << "\nconfig=" << c;
        if (m_tabWidget) {
            oss << "\nNumber of tabs: " << m_tabWidget->count();
            oss << "\nCurrent tab: " << m_tabWidget->currentIndex();
        }
        Log_Info(oss.str());
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::onEditCopy() {
    if (m_project) {
        m_project->copySelection();
        showStatus(tr("Copied to clipboard"), 2000);
    }
}


void mdn::gui::MainWindow::onEditPaste() {
    if (m_project) {
        m_project->pasteOnSelection();
        showStatus(tr("Paste from clipboard"), 2000);
    }
}


void mdn::gui::MainWindow::onEditCut() {
    if (m_project) {
        m_project->cutSelection();
        showStatus(tr("Cut selection"), 2000);
    }
}


void mdn::gui::MainWindow::onEditDelete() {
    if (m_project) {
        m_project->deleteSelection();
        int activeIndex = m_project->activeIndex();
        if (auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(activeIndex))) {
            ndw->update();
        }
        showStatus(tr("Delete selection"), 2000);
    }
}


bool mdn::gui::MainWindow::eventFilter(QObject* watched, QEvent* event) {
    // Block handle drags when snug
    if (m_welded) {
        for (int i = 0; i < m_splitter->count() - 1; ++i) {
            if (watched == m_splitter->handle(i + 1)) {
                switch (event->type()) {
                    case QEvent::MouseButtonPress:
                    case QEvent::MouseMove:
                    case QEvent::MouseButtonRelease:
                        return true; // swallow drag
                    default: break;
                }
            }
        }
    }

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
    if (event->type() == QEvent::WindowStateChange) {
        if (isMaximized() == false && isFullScreen() == false) {
            QTimer::singleShot(
                0,
                this,
                [this]() {
                    applySplitRatio();
                }
            );
        }
    }

    return QMainWindow::eventFilter(watched, event);
}


void mdn::gui::MainWindow::changeEvent(QEvent* e) {
    Log_Debug3_H("");

    if (!m_splitter)
    {
        Log_Debug3_T("");
        QMainWindow::changeEvent(e);
        return;
    }

    // if (e->type() == QEvent::WindowStateChange)
    // {
    //     applySplitRatio();
    // }
    if (e->type() == QEvent::WindowStateChange)
    {
        QTimer::singleShot(0, this, [this]() { applySplitRatio(); });
    }
    else
    {
        if (e->type() == QEvent::Show)
        {
            applySplitRatio();
        }
    }

    Log_Debug3_T("");
    QMainWindow::changeEvent(e);
}


void mdn::gui::MainWindow::closeEvent(QCloseEvent* e) {
    // Ask to save before quitting the app
    if (!confirmedCloseProject(true)) {
        // user hit Cancel or save failed
        e->ignore();
        return;
    }
    // proceed with normal teardown
    QMainWindow::closeEvent(e);
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


void mdn::gui::MainWindow::createCommandWidget() {
    if (enableCommandWidget) {
        m_command = new CommandWidget(this);
        if (m_command) {
            connect(
                m_command,
                &CommandWidget::submitCommand,
                this,
                &MainWindow::onCommandSubmitted
            );
        }
    }
}


void mdn::gui::MainWindow::createMenus() {
    Log_Debug3_H("");
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&New Project", this, &mdn::gui::MainWindow::onNewProject);
    fileMenu->addAction("&Open Project", this, &mdn::gui::MainWindow::onOpenProject);
    fileMenu->addAction("&Save Project", this, &mdn::gui::MainWindow::onSaveProject);
    fileMenu->addAction("&Save Project As...", this, &mdn::gui::MainWindow::onSaveProjectAs);
    fileMenu->addAction("&Close Project", this, &mdn::gui::MainWindow::onCloseProject);
    fileMenu->addSeparator();
    fileMenu->addAction("New Mdn2d", this, &mdn::gui::MainWindow::onNewMdn2d);
    fileMenu->addAction("Open Mdn2d", this, &mdn::gui::MainWindow::onOpenMdn2d);
    fileMenu->addAction("Save Mdn2d", this, &mdn::gui::MainWindow::onSaveMdn2d);
    fileMenu->addAction("Close Mdn2d", this, &mdn::gui::MainWindow::onTabCloseRequested);
    fileMenu->addSeparator();
    fileMenu->addAction("&Project Properties", this, &mdn::gui::MainWindow::onProjectProperties);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", this, &mdn::gui::MainWindow::close);

    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("Select All", this, &mdn::gui::MainWindow::onSelectAll);
    editMenu->addAction("Copy", this, &mdn::gui::MainWindow::onEditCopy);
    editMenu->addAction("Cut", this, &mdn::gui::MainWindow::onEditCut);
    editMenu->addAction("Paste", this, &mdn::gui::MainWindow::onEditPaste);
    editMenu->addSeparator();
    editMenu->addAction("Delete", this, &mdn::gui::MainWindow::onEditDelete);

    // QMenu* toolsMenu = menuBar()->addMenu("&Tools");

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    QAction* actOverview = helpMenu->addAction("&Overview");
    QAction* actGuide = helpMenu->addAction("&Guide");
    QAction* actAbout = helpMenu->addAction("&About");
    QAction* actLicense = helpMenu->addAction("&License");
    connect(actOverview, &QAction::triggered, this, [this]{
        HelpDialog dlg("overview", this);
        dlg.exec();
    });
    connect(actGuide, &QAction::triggered, this, [this]{
        HelpDialog dlg("guide", this);
        dlg.exec();
    });
    connect(actAbout, &QAction::triggered, this, [this]{
        HelpDialog dlg("about", this);
        dlg.exec();
    });
    connect(actLicense, &QAction::triggered, this, [this]{
        HelpDialog dlg("license", this);
        dlg.exec();
    });
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::createToolbars() {
    // ---- Project toolbar
    auto* tbProject = addToolBar(tr("Project"));
    tbProject->setObjectName("tbProject");
    tbProject->setMovable(true);
    tbProject->setIconSize(QSize(18, 18));
    tbProject->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); // << important

    auto iconNew  = QIcon::fromTheme("document-new",  style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    auto iconSave  = QIcon::fromTheme("document-save",  style()->standardIcon(QStyle::SP_DialogSaveButton));
    auto iconOpen  = QIcon::fromTheme("document-open",  style()->standardIcon(QStyle::SP_DialogOpenButton));
    auto iconClose = QIcon::fromTheme("window-close",   style()->standardIcon(QStyle::SP_DialogCloseButton));
    auto iconGear  = QIcon::fromTheme("preferences-system", style()->standardIcon(QStyle::SP_FileDialogDetailedView));

    auto aNew  = tbProject->addAction(iconNew,  tr("New Project"));
    auto aSave  = tbProject->addAction(iconSave,  tr("Save Project"));
    auto aOpen  = tbProject->addAction(iconOpen,  tr("Open Project"));
    auto aClose = tbProject->addAction(iconClose, tr("Close Project"));
    tbProject->addSeparator();
    auto aProps = tbProject->addAction(iconGear,  tr("Properties"));

    aNew->setShortcut(QKeySequence::New);
    aSave->setShortcut(QKeySequence::Save);
    aOpen->setShortcut(QKeySequence::Open);
    aClose->setShortcut(QKeySequence::Close);

    connect(aNew,   &QAction::triggered, this, &MainWindow::onNewProject);
    connect(aSave,  &QAction::triggered, this, &MainWindow::onSaveProject);
    connect(aOpen,  &QAction::triggered, this, &MainWindow::onOpenProject);
    connect(aClose, &QAction::triggered, this, &MainWindow::onCloseProject);
    connect(aProps, &QAction::triggered, this, &MainWindow::onProjectProperties);

    // ---- Edit toolbar
    auto* tbEdit = addToolBar(tr("Edit"));
    tbEdit->setObjectName("tbEdit");
    tbEdit->setMovable(true);
    tbEdit->setIconSize(QSize(18, 18));
    tbEdit->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto iconCopy  = QIcon::fromTheme("edit-copy",  style()->standardIcon(QStyle::SP_DialogYesButton));
    auto iconPaste = QIcon::fromTheme("edit-paste", style()->standardIcon(QStyle::SP_DialogApplyButton));
    auto iconCut   = QIcon::fromTheme("edit-cut",   style()->standardIcon(QStyle::SP_TrashIcon));

    auto actCopy  = tbEdit->addAction(iconCopy,  tr("Copy"));
    auto actPaste = tbEdit->addAction(iconPaste, tr("Paste"));
    auto actCut   = tbEdit->addAction(iconCut,   tr("Cut"));

    actCopy->setShortcut(QKeySequence::Copy);
    actPaste->setShortcut(QKeySequence::Paste);
    actCut->setShortcut(QKeySequence::Cut);

    connect(actCopy,  &QAction::triggered, this, &mdn::gui::MainWindow::onEditCopy);
    connect(actPaste, &QAction::triggered, this, &mdn::gui::MainWindow::onEditPaste);
    connect(actCut,   &QAction::triggered, this, &mdn::gui::MainWindow::onEditCut);
}


void mdn::gui::MainWindow::setupLayout(Mdn2dConfig* cfg) {
    Log_Debug3_H("")

    // Top half - Mdn2d tab widget
    m_tabWidget = new HoverPeekTabWidget(this);

    m_tabWidget->setTabPosition(QTabWidget::South);
    Log_Debug4("");
    auto* bar = m_tabWidget->tabBar();
    bar->setMovable(true);
    // bar->setTabsClosable(true);
    // bar->setMouseTracking(true);
    // connect(bar, &QTabBar::tabBarEntered, this, &MainWindow::onTabHover);

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
    connect(
        bar,
        &QTabBar::tabBarDoubleClicked,
        this,
        &mdn::gui::MainWindow::renameTab
    );
    connect(
        m_tabWidget,
        &HoverPeekTabWidget::changedActiveIndex,
        this,
        &mdn::gui::MainWindow::onChangedActiveIndex
    );

    Log_Debug4("");
    if (!m_project && cfg) {
        Log_Debug3("Dispatch - createNewProject");
        createNewProjectFromConfig(*cfg, false);
    }
    Log_Debug4("");
    // No need to createTabs here, already done via signals

    // Bottom half - command history + input
    m_splitter->addWidget(m_tabWidget);

    // int idx = m_splitter->indexOf(m_command);
    m_ops = new OpsController(this, m_project, m_tabWidget, m_command, this);
    QWidget* container = m_ops->bottomContainer();
    m_splitter->addWidget(container);

    // m_splitter->addWidget(m_command);
    m_splitter->setStretchFactor(0, 3);
    m_splitter->setStretchFactor(1, 1);

    setCentralWidget(m_splitter);

    if (!enableCommandWidget) {
        // m_command->setVisible(false);
        weldSliderToBottom(true);
    }

    Log_Debug3("Dispatch - initOperationsUi");
    initOperationsUi();

    Log_Debug3("Dispatch - initFocusModel");
    initFocusModel();

    Log_Debug3("Dispatch - createToolBars");
    createToolbars();

    Log_Debug3_T("");
}


void mdn::gui::MainWindow::createTabs() {
    Log_Debug2_H("");
    if (!m_project || !m_tabWidget) {
        Log_Debug2_T("No project or tabWidget");
        return;
    }

    std::vector<int> indices;
    indices.reserve(m_project->size());
    for (const auto& kv : m_project->data_addressingIndexToName()) {
        indices.push_back(kv.first);
    }
    std::sort(indices.begin(), indices.end());

    for (int idx : indices) {
        createTabForIndex(idx);
    }

    Log_Debug3("Creating plusTab");
    createPlusTab();
    // updateStatusModeText(m_globalMode);

    // m_tabWidget->setTabsClosable(true);

    // Small corner widget for tab-scope file actions
    auto* corner = new QWidget(m_tabWidget);
    auto* h = new QHBoxLayout(corner);
    h->setContentsMargins(4, 2, 4, 2);
    h->setSpacing(4);

    auto mkBtn = [corner](const char* theme, const QString& tip) {
        auto* b = new QToolButton(corner);
        b->setAutoRaise(true);
        b->setIcon(QIcon::fromTheme(theme));
        b->setToolTip(tip);
        return b;
    };

    auto* btnSave = mkBtn("document-save",  tr("Save current tab"));
    auto* btnOpen = mkBtn("document-open",  tr("Open into new tab"));
    auto* btnZero = mkBtn("edit-clear", tr("Zero all digits"));
    auto* btnClose= mkBtn("window-close",   tr("Close current tab"));

    h->addWidget(btnSave);
    h->addWidget(btnOpen);
    h->addWidget(btnZero);
    h->addWidget(btnClose);
    corner->setLayout(h);
    m_tabWidget->setCornerWidget(corner, Qt::BottomRightCorner);

    connect(btnSave, &QToolButton::clicked, this, &MainWindow::onSaveMdn2d);
    connect(btnOpen, &QToolButton::clicked, this, &MainWindow::onOpenMdn2d);
    connect(btnZero, &QToolButton::clicked, this, &MainWindow::onZeroMdn2d);
    connect(btnClose, &QToolButton::clicked, this, &MainWindow::onTabCloseRequested);
    Log_Debug2_T("");
}


void mdn::gui::MainWindow::createTabForIndex(int index) {
    Log_Debug3_H("index=" << index);
    Mdn2d* src = m_project->getMdn(index);
    if (!src) {
        Log_WarnQ("Failed to acquire Mdn for index " << index << ", cannot continue.");
        return;
    }

    Selection& sel = src->selection();
    auto* ndw = new NumberDisplayWidget;
    ndw->setProject(m_project);
    ndw->setFocusPolicy(Qt::StrongFocus);
    ndw->setModel(src, &sel);
    ndw->setFontPointSize(m_globalFontSize);

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
    connect(
        ndw,
        &NumberDisplayWidget::requestCycleFraxis,
        this,
        &MainWindow::cycleFraxis
    );
    connect(
        ndw,
        &NumberDisplayWidget::requestCycleSignConvention,
        this,
        &MainWindow::cycleSignConvention
    );
    connect(
        ndw,
        &NumberDisplayWidget::requestStatus,
        this,
        &MainWindow::showStatus
    );
    connect(
        ndw,
        &NumberDisplayWidget::cancelRequested,
        this,
        &MainWindow::onCancelRequested
    );

    // Push current global mode into the new widget
    ndw->setEditMode(m_globalMode);

    connect(
        ndw,
        &NumberDisplayWidget::requestFontSizeChange,
        this,
        &MainWindow::setGlobalFontSize
    );
    connect(
        m_project,
        &Project::mdnContentChanged,
        ndw,
        qOverload<>(&mdn::gui::NumberDisplayWidget::update)
    );

    std::string name = src->name();
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

    setActiveTab(tab);
    ndw->setFocus();
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::createPlusTab() {
    Log_Debug3_H("");
    const int plusIdx = m_tabWidget->count();

    // Ensure plus tab does not exist
    if (hasPlusTab()) {
        Log_Debug3_T("Already has a plusTab");
        return;
    }

    auto* marker = new MarkerWidget;
    m_tabWidget->addTab(marker, MdnQtInterface::toQString("+"));
    // QWidget* dummy = new QWidget;
    // m_tabWidget->addTab(dummy, MdnQtInterface::toQString("+"));

    m_tabWidget->setPlusTab(plusIdx, marker);
    m_plusTab = true;
    Log_Debug3_T("");
}


bool mdn::gui::MainWindow::hasPlusTab() {
    // #ifdef MDN_DEBUG
    //     Log_Debug4_H("");
    //     if (!m_tabWidget) {
    //         if (m_plusTab) {
    //             Log_WarnQ("Meta data out-of-sync: no plusTab exists but flag is true");
    //         }
    //         m_plusTab = false;
    //         Log_Debug4_T("No tab widget, returning false, setting m_plusTab = false");
    //         return false;
    //     }

    //     const int plusIdx = m_tabWidget->count() - 1;
    //     if (plusIdx < 0) {
    //         // No tabs, including the plusTab, i.e. no plusTab
    //         if (m_plusTab) {
    //             Log_WarnQ("Meta data out-of-sync: plusTab missing but flag is true");
    //             m_plusTab = false;
    //             Log_Debug4("m_plusTab=" << m_plusTab << ", set to false");
    //         }
    //         Log_Debug4_T("No tabs, returning false");
    //         return false;
    //     }
    //     auto* mark = qobject_cast<MarkerWidget*>(m_tabWidget->widget(plusIdx));
    //     if (mark) {
    //         // Cast succeeded, plusTab is here
    //         if (!m_plusTab) {
    //             Log_WarnQ(
    //                 "Meta data out-of-sync: plusTab exists but flag is false"
    //                     << ", last tab is [" << m_tabWidget->tabText(plusIdx).toStdString()
    //                     << "]"
    //             );
    //             Log_Debug4("Setting m_plusTab = true");
    //             m_plusTab = true;
    //         }
    //         Log_Debug4_T("Last tab has a MarkerWidget");
    //         return true;
    //     }
    //     auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(plusIdx));
    //     if (ndw) {
    //         Log_WarnQ(
    //             "It's an NDW, named [" << m_tabWidget->tabText(plusIdx).toStdString() << "]"
    //         );
    //     }
    //     if (m_plusTab) {
    //         Log_WarnQ("Meta data out-of-sync: plusTab missing but flag is true");
    //         Log_Debug4("Setting m_plusTab = false");
    //         m_plusTab = false;
    //     }
    //     Log_Debug4_T("No plusTab");
    //     return false;
    // #endif
    return m_plusTab;
}


void mdn::gui::MainWindow::removePlusTab() {
    const int plusIdx = m_tabWidget->count()-1;

    QWidget* w = m_tabWidget->widget(plusIdx);
    m_tabWidget->removeTab(plusIdx);
    delete w;
    w = nullptr;
    m_tabWidget->setPlusTab(-1, nullptr);
    m_plusTab = false;
    Log_Debug4("m_plusTab=" << m_plusTab << ", set to false");
}


void mdn::gui::MainWindow::initOperationsUi() {
    if (m_ops) {
        m_strip = m_ops->strip();
        m_status = m_ops->status();
        connect(m_ops, &OpsController::planReady, this, &MainWindow::onOpsPlan);
        connect(
            m_ops,
            &OpsController::tabClicked,
            this,
            &mdn::gui::MainWindow::onTabCommit
        );

        connect(
            m_ops,
            &OpsController::plusClicked,
            this,
            &mdn::gui::MainWindow::onNewMdn2d
        );
        connect(
            m_ops,
            &OpsController::divisionIterateRequested,
            this,
            &mdn::gui::MainWindow::onDivisionIterateRequested
        );
        connect(
            m_ops,
            &OpsController::divisionStopRequested,
            this,
            &mdn::gui::MainWindow::onDivisionStopRequested
        );
        connect(
            m_strip,
            &OperationStrip::transposeClicked,
            this,
            &mdn::gui::MainWindow::onTransposeClicked
        );
        connect(
            m_strip,
            &OperationStrip::carryOverClicked,
            this,
            &mdn::gui::MainWindow::onCarryOverClicked
        );
        connect(
            m_strip,
            &OperationStrip::carryPosClicked,
            this,
            &mdn::gui::MainWindow::onCarryPosClicked
        );
        connect(
            m_strip,
            &OperationStrip::carryNegClicked,
            this,
            &mdn::gui::MainWindow::onCarryNegClicked
        );
    }
    auto* s = m_ops->status();
    if (s) {
        connect(
            s,
            &StatusDisplayWidget::contentHeightChanged,
            this,
            [this](int){
                QMetaObject::invokeMethod(
                    this,

                    "onRequestFitBottomToContents",
                    Qt::QueuedConnection
                );
            }
        );
        QMetaObject::invokeMethod(
            this,
            "onRequestFitBottomToContents",
            Qt::QueuedConnection
        );
    }
}


void mdn::gui::MainWindow::createStatusBar()
{
    // Build status bar, from left to right
    auto* sb = statusBar();

    // ~~~ Status mode button
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
    // right side arrow; right-click opens menu
    m_statusModeBtn->setPopupMode(QToolButton::MenuButtonPopup);
    sb->addPermanentWidget(m_statusModeBtn);

    // ~~~ Fraxis button
    m_statusFraxisBtn = new QToolButton(this);
    m_statusFraxisBtn->setAutoRaise(true);
    m_statusFraxisBtn->setToolTip(tr("Fraxis (click to toggle; right-click to choose)"));
    m_statusFraxisBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);

    // ~~~ SignConvention button
    m_statusSignConventionBtn = new QToolButton(this);
    m_statusSignConventionBtn->setAutoRaise(true);
    m_statusSignConventionBtn->setToolTip(
        tr("SignConvention (click to toggle; right-click to choose)")
    );
    m_statusSignConventionBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);

    // Seed initial label from project config
    if (m_project) {
        updateStatusFraxisText(m_globalConfig.fraxis());
        updateStatusSignConventionText(m_globalConfig.signConvention());
    } else {
        updateStatusFraxisText(Fraxis::X);
        updateStatusSignConventionText(SignConvention::Positive);
    }

    // Context menu to pick X or Y / Positive, Neutral or Negative
    buildFraxisMenu();
    buildSignConventionMenu();

    // Left-click: toggle X ↔ Y
    connect(
        m_statusFraxisBtn,
        &QToolButton::clicked,
        this,
        &MainWindow::cycleFraxis
    );

    // Left-click: toggle positive → neutral → negative
    connect(
        m_statusSignConventionBtn,
        &QToolButton::clicked,
        this,
        &MainWindow::cycleSignConvention
    );

    sb->addPermanentWidget(m_statusFraxisBtn);
    sb->addPermanentWidget(m_statusSignConventionBtn);

    // ~~~ Cursor and selection text
    m_statusCursor = new QLabel(this);
    m_statusSel    = new QLabel(this);

    m_statusCursor->setText("(0,0)");
    m_statusSel->setText("(empty)");

    // Left to right; make them compact
    sb->addPermanentWidget(m_statusCursor, 0);
    sb->addPermanentWidget(new QLabel("  |  ", this), 0);
    // stretch last one a bit
    sb->addPermanentWidget(m_statusSel, 1);
}


bool mdn::gui::MainWindow::createNewProjectFromConfig(Mdn2dConfig& cfg, bool requireConfirm) {
    Log_Debug3_H("cfg=" << cfg);
    if (!confirmedCloseProject(requireConfirm)) {
        Log_Debug3_T("Failed to close existing project, cannot continue");
        return false;
    }
    m_globalConfig = cfg;
    m_project = new Project(this, cfg, nStartMdnDefault);
    if (m_project) {
        connect(m_project, &mdn::gui::Project::tabsAboutToChange,
                this, &mdn::gui::MainWindow::onProjectTabsAboutToChange);
        connect(m_project, &mdn::gui::Project::tabsChanged,
                this, &mdn::gui::MainWindow::onProjectTabsChanged);
        if (m_ops) {
            m_ops->resetModel(m_project);
            connect(m_ops, &OpsController::requestStatus,
                this, &mdn::gui::MainWindow::showStatus
            );
            connect(m_ops, &OpsController::requestClearStatus,
                this, &mdn::gui::MainWindow::clearStatus
            );
            connect(m_ops, &OpsController::requestFitBottomToContents,
                this, &mdn::gui::MainWindow::onRequestFitBottomToContents
            );
        }
        onProjectTabsChanged(0);
    }
    m_globalConfig.setParent(*m_project);
    setGlobalConfig(m_globalConfig, true);

    Log_Debug3_T("");
    return true;
}


bool mdn::gui::MainWindow::createNewProject(Mdn2dConfig* cfg, bool requireConfirm) {
    Log_Debug3_H("");

    if (!confirmedCloseProject(requireConfirm)) {
        Log_Debug3_T("Did not successfully close existing project");
        return false;
    }

    m_project = new Project(this);
    m_globalConfig.setParent(*m_project);
    doProjectProperties();
    if (m_project) {
        connect(m_project, &mdn::gui::Project::tabsAboutToChange,
                this, &mdn::gui::MainWindow::onProjectTabsAboutToChange);
        connect(m_project, &mdn::gui::Project::tabsChanged,
                this, &mdn::gui::MainWindow::onProjectTabsChanged);
        if (m_ops) {
            m_ops->resetModel(m_project);
            connect(m_ops, &OpsController::requestStatus,
                this, &mdn::gui::MainWindow::showStatus
            );
            connect(m_ops, &OpsController::requestClearStatus,
                this, &mdn::gui::MainWindow::clearStatus
            );
            connect(m_ops, &OpsController::requestFitBottomToContents,
                this, &mdn::gui::MainWindow::onRequestFitBottomToContents
            );
        }
        onProjectTabsChanged(0);
    }
    Log_Debug3_T("");
    return true;
}


bool mdn::gui::MainWindow::openProject(bool requireConfirm) {
    Log_Debug2_H("");

    QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    QString path = QFileDialog::getOpenFileName(
        this,
        tr("Open Project"),
        docs,
        tr("MDN Project (*.mdnproj);;All Files (*)")
    );

    if (path.isEmpty()) {
        showStatus(tr("Load cancelled"), 2000);
        Log_Debug2_T("user cancelled")
        return false;
    }

    if (!confirmedCloseProject(requireConfirm)) {
        showStatus(tr("Load cancelled"), 2000);
        Log_Debug2_T("Did not succeed in closing existing project");
        return false;
    }

    std::unique_ptr<Project> ptr = Project::loadFromFile(this, path.toStdString());
    if (!ptr.get()) {
        // Load failed
        showStatus(tr("Failed to read file"), 2000);
        Log_Debug2_T("Read failed");
        return false;
    }
    m_project = ptr.release();

    connect(m_project, &mdn::gui::Project::tabsAboutToChange,
            this, &mdn::gui::MainWindow::onProjectTabsAboutToChange);
    connect(m_project, &mdn::gui::Project::tabsChanged,
            this, &mdn::gui::MainWindow::onProjectTabsChanged);


    // Remember the folder path of the opened project
    QFileInfo info(path);
    QString folder = info.absolutePath();
    QString baseName = info.baseName();
    m_project->setPath(folder.toStdString());
    m_project->setName(baseName.toStdString());
    setWindowTitle(baseName);

    onProjectTabsChanged(m_project->activeIndex());
    m_globalConfig.setParent(*m_project);
    setGlobalConfig(m_globalConfig);
    showStatus(tr("Project loaded"), 2000);
    Log_Debug2_T("ok")
    return true;
}


bool mdn::gui::MainWindow::confirmedCloseProject(bool requireConfirm) {
    Log_Debug2_H("");
    if (!m_project) {
        Log_Debug2_T("Project already closed");
        showStatus(tr("Cannot close project - no active project"), 2000);
        return true;
    }
    if (!requireConfirm) {
        bool result = closeProject();
        Log_Debug2_T("Closed project, result=" << result);
        return true;
    }

    // Now seek confirmation from user
    std::ostringstream oss;
    oss << "Unsaved changes may be lost.\n\n";
    oss << "Do you want to save changes to project '" << m_project->name();
    oss << "' before proceeding?";
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Save Project",
        oss.str().c_str(),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save // Default button
    );
    if (reply == QMessageBox::Save) {
        bool success = onSaveProject();
        if (!success) {
            // User chose to save, but saving failed.  Do not proceed.
            Log_Debug2_T("Failed to save on request");
            return false;
        }
    } else if (reply == QMessageBox::Cancel) {
        showStatus(tr("Cancelled 'Close project' operation"), 2000);
        Log_Debug2_T("User cancelled when asked to save");
        return false;
    }
    // User probably chose Discard
    Log_Debug3("Deleting existing project");
    bool result = closeProject();
    showStatus(tr("Project closed"), 2000);
    Log_Debug2_T("result = " << result);
    return result;
}


bool mdn::gui::MainWindow::closeProject() {
    Log_Debug2_H("");
    if (!m_project) {
        Log_Debug2_T("no project, returning false");
        return false;
    }
    if (m_tabWidget) {
        QSignalBlocker blockTabs(m_tabWidget);

        // Delete all tabs, including 'plusTab'
        const int n = m_tabWidget->count();
        for (int i = n - 1; i >= 0; --i) {
            QWidget* w = m_tabWidget->widget(i);
            m_tabWidget->removeTab(i);
            delete w;
        }
        m_plusTab = false;
        Log_Debug4("All deleted, m_plusTab=" << m_plusTab << ", set to false");
    }

    disconnect(m_project, nullptr, this, nullptr);

    delete m_project;
    m_project = nullptr;

    QString m(tr("MDN Editor"));
    setWindowTitle(QStringLiteral("%1").arg(m));
    if (m_status) {
        m_status->clearMessage();
    }
    if (statusBar()) {
        statusBar()->clearMessage();
    }

    focusActiveGrid();

    Log_Debug2_T("project deleted, returning true");
    return true;
}


bool mdn::gui::MainWindow::createNewMdn2d(QString name, int index, bool makeActive) {
    std::string nameStd = name.toStdString();
    Log_Debug2_H(
        "Attempt to create {'" << nameStd << "'," << index << "}, makeActive=" << makeActive
    );
    if (!m_project) {
        Log_Debug2_T("No project, cannot create Mdn2d");
        showStatus(tr("No project, cannot create tab"), 2000);
        return false;
    }
    Mdn2d num = Mdn2d::NewInstance(m_globalConfig, name.toStdString());
    std::string actualName = num.name();
    Log_Debug3("Created Mdn2d '" << nameStd << "', actual assigned name='" << actualName << "'");
    // Insertion will trigger a tabWidget update
    QString actualNameQ(MdnQtInterface::toQString(actualName));
    if (nameStd.size() && actualName != nameStd) {
        showStatus(tr("Tab '%1' created (requested '%2')").arg(actualNameQ).arg(name), 2000);
    } else {
        showStatus(tr("Tab '%1' created").arg(actualNameQ), 2000);
    }
    removePlusTab();
    m_project->insertMdn(std::move(num), index);
    int actualIndex = m_project->indexOfMdn(actualName);
    if (makeActive) {
        setActiveTab(actualIndex);
    }
    if (index != actualIndex || nameStd != actualName) {
        Log_Debug3(
            "Attempted to create: {'" << nameStd << "'," << index << "}, "
                << "actual result: {'" << actualName << "'," << actualIndex << "}"
        );
    }
    createPlusTab();
    Log_Debug2_T("");
    return true;
}


void mdn::gui::MainWindow::centreView(int index) {
    auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(index));
    ndw->armCentreViewOnOrigin();
}


void mdn::gui::MainWindow::doProjectProperties() {
    Log_Debug2_H("");
    if (!m_project) {
        Log_Debug2_T("");
        return;
    }

    ProjectPropertiesDialog dlg(m_project, this);
    // You may show a path hint if you have one (or keep empty)
    dlg.setInitial(
        QString::fromStdString(m_project->name()),
        QString::fromStdString(m_project->path()),
        m_globalConfig
    );
    if (dlg.exec() != QDialog::Accepted) {
        showStatus(tr("Cancelled changes to properties"), 2000);
        Log_Debug2_T("User rejected change");
        return;
    }

    m_project->setName(MdnQtInterface::fromQString(dlg.projectName()));
    setWindowTitle(dlg.projectName());
    Log_Debug3_H("setGlobalConfig dispatch");
    setGlobalConfig(dlg.chosenConfig());
    Log_Debug3_T("setGlobalConfig return");
    showStatus(tr("Updated project properties"), 2000);
    Log_Debug2_T("Done projectProperties window");
    // If tabs changed due to clearing or other effects, your existing logic keeps UI in sync.
}


void mdn::gui::MainWindow::buildFraxisMenu() {
    m_fraxisMenu = new QMenu(this);
    m_fraxisMenu->addAction("X", this, &MainWindow::chooseFraxisX);
    m_fraxisMenu->addAction("Y", this, &MainWindow::chooseFraxisY);
    m_statusFraxisBtn->setMenu(m_fraxisMenu);
    // right side arrow; right-click opens menu
    m_statusFraxisBtn->setPopupMode(QToolButton::MenuButtonPopup);
}


void mdn::gui::MainWindow::buildSignConventionMenu() {
    m_signConventionMenu = new QMenu(this);
    m_signConventionMenu->addAction(
        "[+] Positive", this, &MainWindow::chooseSignConventionPositive
    );
    m_signConventionMenu->addAction(
        "[o] Neutral", this, &MainWindow::chooseSignConventionNeutral
    );
    m_signConventionMenu->addAction(
        "[-] Negative", this, &MainWindow::chooseSignConventionNegative
    );
    m_statusSignConventionBtn->setMenu(m_signConventionMenu);
    // right side arrow; right-click opens menu
    m_statusSignConventionBtn->setPopupMode(QToolButton::MenuButtonPopup);
}


void mdn::gui::MainWindow::updateStatusFraxisText(Fraxis f) {
    Log_Debug2_H("f=" << f);
    if (!m_statusFraxisBtn) {
        Log_Debug2_T("button missing");
        return;
    }
    const QString text = (f == Fraxis::Y) ? QStringLiteral("FY") : QStringLiteral("FX");
    m_statusFraxisBtn->setText(text);

    // reflect state in the popup menu (if present)
    switch (f) {
        case Fraxis::X: {
            Log_Debug4("Setting fraxis status to 'X'");
            m_statusFraxisBtn->setText("X");
            showStatus(tr("Fraxis >> X"), 2000);
            break;
        }
        case Fraxis::Y: {
            Log_Debug4("Setting fraxis status to 'Y'");
            m_statusFraxisBtn->setText("Y");
            showStatus(tr("Fraxis >> Y"), 2000);
            break;
        }
        default: {
            Log_Warn("Setting fraxis status to '?'");
            m_statusFraxisBtn->setText("?");
            showStatus(tr("Fraxis >> ?"), 2000);
            break;
        }
    }
    Log_Debug2_T("");
}


void mdn::gui::MainWindow::updateStatusSignConventionText(SignConvention sc) {
    Log_Debug2_H("sc=" << sc);
    if (!m_statusSignConventionBtn) {
        Log_Debug2_T("button missing");
        return;
    }

    QString text;
    switch (sc) {
        case SignConvention::Positive:
            text = "+ Pos";
            break;
        case SignConvention::Neutral:
            text = "+ / -";
            break;
        case SignConvention::Negative:
            text = "Neg -";
            break;
    }
    m_statusSignConventionBtn->setText(text);
    std::string st("SignConvention >> " + SignConventionToName(sc));
    QString qst(QString::fromStdString(st));
    showStatus(qst, 2000);

    Log_Debug2_T("");
}


void mdn::gui::MainWindow::onTabMoved(int from, int to) {
    Log_Debug3_H("from " << from << " to " << to);
    bool hasPlus = hasPlusTab();
    // Log_InfoQ("from " << from << " to " << to << ", with hasPlusTab=" << hasPlus);
    if (!m_tabWidget) {
        Log_Debug3_T("No tabWidget");
        return;
    }
    if (hasPlus) {
        const int plusTab = m_tabWidget->count() - 1;
        if (from >= plusTab || to >= plusTab) {
            Log_WarnQ("Illegal move across '+': refreshing from model");
            syncTabsToProject();
            return;
        }
    }
    m_project->moveMdn(from, to);
    syncTabsToProject();
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::onTabCloseRequested() {
    Log_Debug3_H("");
    closeTab(-1);
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::closeTab(int index) {
    Log_Debug3_H("");
    if (!m_project) {
        Log_Debug3_T("No project");
        return;
    }
    if (index < 0) {
        index = m_tabWidget ? m_tabWidget->currentIndex() : m_project->activeIndex();
    } else if (m_tabWidget && index == m_tabWidget->count() - 1 && hasPlusTab()) {
        Log_WarnQ("Attempting to 'closeTab' the 'plusTab', skipping step");
        Log_Debug3_T("cannot 'closeTab' 'plusTab'");
        return;
    }
    Mdn2d* tgt = m_project->getMdn(index);
    if (!tgt->bounds().empty()) {
        std::ostringstream oss;
        oss << "Unsaved data on tab '" << tgt->name() << "' will be lost.\n\n"
            << "Are you sure?";
        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Overwrite Number?"),
            tr(oss.str().c_str()),
            QMessageBox::Close | QMessageBox::Cancel,
            QMessageBox::Cancel // Default button
        );
        if (reply != QMessageBox::Close) {
            showStatus(tr("Cancelled 'Close tab' operation"), 2000);
            Log_Debug3_T("User rejected close operation");
            return;
        }
    }
    // User said 'Close'
    m_project->deleteMdn(index);
    showStatus(tr("Tab closed"), 2000);
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
    showStatus(tr("Duplicated tab"), 2000);
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
    showStatus(tr("Copy tab to clipboard"), 2000);
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::pasteTab(int insertAt)
{
    Log_Debug3_H("");
    if (!m_project) {
        showStatus(tr("No project: Cannot paste tab"), 2000);
        Log_Debug3_T("");
        return;
    }
    m_project->pasteOnSelection(insertAt);
    showStatus(tr("Tab pasted (at %1) from clipboard").arg(insertAt), 2000);
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::syncTabsToProject() {
    Log_Debug3_H("");
    if (!m_project) {
        Log_Debug3_T("No project");
        return;
    }
    // No need to account for 'plusTab' as cast will fail
    const int n = m_tabWidget->count();
    for (int i = 0; i < n; ++i) {
        auto* view = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(i));

        if (!view) {
            continue;
        }
        view->setProject(m_project);
        Mdn2d* srcNum = m_project->getMdn(i);
        if (!srcNum) {
            continue;
        }
        Selection& srcSel = srcNum->selection();

        view->setModel(srcNum, &srcSel);
        // add mdnName(int)
        m_tabWidget->setTabText(i, QString::fromStdString(srcNum->name()));
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::setActiveTab(int index) {
    if (!m_project || !m_tabWidget) {
        return;
    }
    m_project->setActiveMdn(index);
    m_tabWidget->setCurrentIndex(index);
}


void mdn::gui::MainWindow::fitBottomToContents() {
    Log_Debug3_H("");
    if (!m_splitter || !m_ops || !m_ops->bottomContainer()) {
        Log_Debug3_T(""
            << "Missing component "
            << "(splitter:" << (m_splitter?"Okay":"MISSING")
            << ", ops:" << (m_ops?"Okay":"MISSING")
            << ", bottomContainer:" << (m_ops->bottomContainer()?"Okay":"MISSING")
            << ")"
        );
        return;
    }
    QWidget* bottom = m_ops->bottomContainer();

    // 1) Fix the bottom height to its sizeHint (prevents stretching/collapse)
    const int want = bottom->sizeHint().height();
    bottom->setMinimumHeight(want);
    bottom->setMaximumHeight(want);

    // 2) Give the bottom exactly what it wants, top gets the rest
    QList<int> sizes = m_splitter->sizes();
    int total = 0; for (int s : sizes) total += s;
    if (total <= 0) total = height();

    sizes.resize(2);
    sizes[1] = want;
    sizes[0] = std::max(0, total - want);
    m_splitter->setSizes(sizes);
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::releaseBottomWeld()
{
    Log_Debug3_H("");
    if (!m_ops || !m_ops->bottomContainer()) {
        Log_Debug3_T("Missing components");
        return;
    }
    QWidget* bottom = m_ops->bottomContainer();
    bottom->setMinimumHeight(0);
    bottom->setMaximumHeight(QWIDGETSIZE_MAX);
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::weldSliderToBottom(bool on)
{
    Log_Debug3_H("on=" << on);
    m_welded = on;
    if (!m_splitter) {
        Log_Debug3_T("Missing components");
        return;
    }

    // Install/remove filters on all handles (vertical splitter => 1 handle)
    for (int i = 0; i < m_splitter->count() - 1; ++i) {
        auto* h = m_splitter->handle(i + 1);
        if (!h) {
            continue;
        }
        if (on) {
            h->installEventFilter(this);
        } else {
            h->removeEventFilter(this);
        }
    }

    if (on) {
        QTimer::singleShot(0, this, [this]{ fitBottomToContents(); });
    } else {
        releaseBottomWeld();
    }
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::updateStatusModeText(NumberDisplayWidget::EditMode m) {
    std::string mStr(NumberDisplayWidget::EditModeToString(m));
    Log_Debug3_H(mStr);
    if (!m_statusModeBtn) {
        Log_Debug3_T("Missing statusMode pieces");
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
    showStatus(tr("Edit mode >> %1").arg(MdnQtInterface::toQString(mStr)), 2000);
    Log_Debug3_T("");
}


void mdn::gui::MainWindow::updateStatusSelectionText(const Selection& s) {
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
    Log_Debug4_H("");

    QWidget* w = activeGridWidget();
    if (w && m_project) {
        Log_Debug4("Setting focus");
        w->setFocus(Qt::ShortcutFocusReason);
    }
    Log_Debug4_T("");
}


QWidget* mdn::gui::MainWindow::activeGridWidget() const
{
    QWidget* w{nullptr};

    if (m_tabWidget && m_project && m_project->size()) {
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


void mdn::gui::MainWindow::ensureTabCorner() {
    if (!m_tabWidget) return;

    if (!m_tabCorner) {
        m_tabCorner = new QWidget(m_tabWidget);
        auto* h = new QHBoxLayout(m_tabCorner);
        h->setContentsMargins(4,2,4,2);
        h->setSpacing(4);

        auto mkBtn = [this](const QIcon& ic, const QString& tip){
            auto* b = new QToolButton(m_tabCorner);
            b->setAutoRaise(true);
            b->setIcon(ic);
            b->setToolTip(tip);
            return b;
        };

        auto icSave = QIcon::fromTheme(
            "document-save",
            style()->standardIcon(QStyle::SP_DialogSaveButton)
        );
        auto icOpen = QIcon::fromTheme(
            "document-open",
            style()->standardIcon(QStyle::SP_DialogOpenButton)
        );
        auto icZero = QIcon::fromTheme(
            "clear-all",
            style()->standardIcon(QStyle::SP_DialogDiscardButton)
        );
        auto icClose = QIcon::fromTheme(
            "window-close",
            style()->standardIcon(QStyle::SP_DialogCloseButton)
        );

        m_tabSaveBtn  = mkBtn(icSave,  tr("Save current tab"));
        m_tabOpenBtn  = mkBtn(icOpen,  tr("Open into new tab"));
        m_tabZeroBtn = mkBtn(icZero, tr("Zero all digits"));
        m_tabCloseBtn = mkBtn(icClose, tr("Close current tab"));

        h->addWidget(m_tabSaveBtn);
        h->addWidget(m_tabOpenBtn);
        h->addWidget(m_tabZeroBtn);
        h->addWidget(m_tabCloseBtn);

        connect(m_tabSaveBtn, &QToolButton::clicked, this, &MainWindow::onSaveMdn2d);
        connect(m_tabOpenBtn, &QToolButton::clicked, this, &MainWindow::onOpenMdn2d);
        connect(m_tabZeroBtn, &QToolButton::clicked, this, &MainWindow::onZeroMdn2d);
        connect(m_tabCloseBtn, &QToolButton::clicked, this, &MainWindow::onTabCloseRequested);
    }

    // (Re)attach – safe to call many times
    m_tabWidget->setCornerWidget(m_tabCorner, Qt::BottomRightCorner);
    m_tabCorner->show();
}


void mdn::gui::MainWindow::divide(const OperationPlan& p, Mdn2d& a, Mdn2d& b) {
    // First - acquire references to destination and remainder
    Log_Debug2_H("p=" << p << ", a=[" << a.name() << "],b=[" << b.name() << "]");
    Mdn2d* destPtr(nullptr);
    Mdn2d* remPtr(nullptr);
    int answerIndex;
    if (p.indexDest >= 0) {
        Log_Debug3("Getting destination " << p.indexDest);
        destPtr = m_project->getMdn(p.indexDest);
        answerIndex = p.indexDest;
    } else {
        std::string pNameStr = p.newName.toStdString();
        Log_Debug3("Creating new destination [" << pNameStr << "]");
        Mdn2d newDest(m_globalConfig, pNameStr);
        int destIndex = m_project->activeIndex() + 1;
        m_project->insertMdn(std::move(newDest), destIndex);
        destPtr = m_project->getMdn(destIndex);
        answerIndex = destIndex;
    }
    if (p.indexRem >= 0) {
        Log_Debug3("Getting remainder " << p.indexRem);
        remPtr = m_project->getMdn(p.indexRem);
    } else {
        Log_Debug3("Getting remainder " << p.indexRem);
        std::string pRemNameStr = p.newRemName.toStdString();
        Log_Debug3("Creating new remainder [" << pRemNameStr << "]");
        Mdn2d newRem(m_globalConfig, pRemNameStr);
        int remIndex = m_project->activeIndex() + 1;
        m_project->insertMdn(std::move(newRem), remIndex);
        remPtr = m_project->getMdn(remIndex);
    }
    // Set activeIndex to answer
    if (m_project && m_tabWidget) {
        m_project->setActiveMdn(answerIndex);
        m_tabWidget->setCurrentSelectedIndex(answerIndex);
    }

    Mdn2d& dest = *destPtr;
    Mdn2d& rem = *remPtr;
    rem.clear();
    rem += a;

    // Next, fly the division algorithm

    long double remMag = constants::ldoubleGreat;
    Log_Debug3_H("divideIterate dispatch");
    a.divideIterate(10, b, dest, rem, remMag, m_globalConfig.fraxis());
    Log_Debug3_T("divideIterate return");
    if (remMag > 0.0) {
        Log_Debug4("remMag non-zero:" << remMag);
        m_ops->enterActiveDivision();
        m_ad_operandA = &a;
        m_ad_operandB = &b;
        m_ad_remainder = &rem;
        m_ad_destination = &dest;
        m_ad_remMag = remMag;
        showStatus(tr("Division underway - press [÷] to continue, or [Cancel]"), 0);
    } else {
        Log_Debug4("remMag is zero");
        clearStatus();
        showStatus(tr("Calculation complete"), 2000);
    }
    syncTabsToProject();
    setActiveTab(p.indexA);
    if (m_project) {
        int activeIndex = m_project->activeIndex();
        if (auto* ndw = qobject_cast<NumberDisplayWidget*>(m_tabWidget->widget(activeIndex))) {
            ndw->update();
        }
    }
    Log_Debug2_T("");
    return;
}
