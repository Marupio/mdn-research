#pragma once

#include <QAction>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "CommandWidget.hpp"
#include "NumberDisplayWidget.hpp"
#include "OpsController.hpp"
#include "Project.hpp"


#include "../library/Mdn2d.hpp"

namespace mdn {
namespace gui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    // Create null, no project loaded
    MainWindow(QWidget *parent);

    // Create with a project, given an *accepted* dialogue
    MainWindow(QWidget *parent, Mdn2dConfig* cfg);

    // Destructor - utter destruction
    ~MainWindow() override;

signals:
    void newMdn2dRequested();

private slots:
    void onTabContextMenu(const QPoint& pos);
    void onProjectTabsAboutToChange();
    void onProjectTabsChanged(int currentIndex);
    void onProjectProperties();

    // File menu operations
    bool onNewProject();
    bool onNewMdn2d();
    bool onNewNamedMdn2d(QString name, int index);
    bool onSaveProject();
    bool onOpenProject();
    bool onSaveMdn2d(int idx = -1);
    bool onOpenMdn2d();
    bool onCloseProject();

    void onSplitterMoved(int pos, int index);
    void onCommandSubmitted(const QString& text);
    void onOpsPlan(const OpsController::Plan& p);

    void cycleEditMode();
    void setGlobalEditMode(NumberDisplayWidget::EditMode m);
    void toggleGlobalEditMode(NumberDisplayWidget::EditMode m);
    void cycleGlobalEditMode(bool forward);
    void onEditModeChanged(NumberDisplayWidget::EditMode m);

    void chooseModeOverwrite();
    void chooseModeAdd();
    void chooseModeSubtract();

    void setGlobalFontSize(int pt);
    void setGlobalConfig(Mdn2dConfig c, bool force=false);

    // Fraxis control
    void cycleFraxis();
    void chooseFraxisX();
    void chooseFraxisY();

    void slotSelectNextTab();
    void slotSelectPrevTab();
    void slotMoveTabRight();
    void slotMoveTabLeft();
    void slotDebugShowAllTabs();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* e) override;

private:
    void createSplitter();
    void createMenus();
    void setupLayout(Mdn2dConfig* cfg=nullptr);
    void createTabs();
    void createTabForIndex(int index);
    void initOperationsUi();
    void createStatusBar();

    // Sets m_project to a new project based on the given config
    bool createNewProjectFromConfig(Mdn2dConfig& cfg, int nStartMdn=3);

    // File menu operations (full implementations)
    // Create a new project, confirms close of existing project
    bool createNewProject(Mdn2dConfig* cfg=nullptr);
    // Returns true if m_project has been successfully closed
    bool confirmedCloseProject();
    // Does the dirty work of actually closing the project
    bool closeProject();
    // Creates a new Mdn2d, inserts into the project, adds a tab
    bool createNewMdn2d(QString name, int index, bool makeActive);

    // ProjectProperties window
    void doProjectProperties();
    void absorbProjectProperties(ProjectPropertiesDialog* dlg);

    // Fraxis helpers
    void updateStatusFraxisText(mdn::Fraxis f);
    void buildFraxisMenu();

    // Tab operations
    void onTabMoved(int from, int to);
    void onTabCloseRequested(int index);
    void renameTab(int index);
    void duplicateTab(int index);
    void copyTab(int index);
    void pasteTab(int insertAt);
    void syncTabsToProject();
    void setActiveTab(int index);

    // Status bar
    void updateStatusModeText(NumberDisplayWidget::EditMode m);
    void updateStatusSelectionText(const mdn::Selection& s);

    // Focus model API
    void initFocusModel();
    void focusActiveGrid();
    QWidget* activeGridWidget() const;
    bool isFocusAllowedWidget(QWidget* w) const;
    void onAppFocusChanged(QWidget* old, QWidget* now);
    void applySplitRatio();

    QSplitter* m_splitter = nullptr;
    double m_splitRatio{0.5};
    NumberDisplayWidget::EditMode m_globalMode = NumberDisplayWidget::EditMode::Overwrite;

    // MDN Digit Browser (upper pane)
    QTabWidget* m_tabWidget = nullptr;

    // Command History (lower pane)
    CommandWidget* m_command = nullptr;

    QLabel* m_statusCursor = nullptr;
    QLabel* m_statusSel = nullptr;
    QToolButton* m_statusModeBtn = nullptr;
    QMenu* m_modeMenu = nullptr;
    QToolButton* m_statusFraxisBtn{nullptr};
    QMenu*       m_fraxisMenu{nullptr};

    int m_globalFontSize = 11;
    Mdn2dConfig m_globalConfig;

    OpsController* m_ops{nullptr};
    Project* m_project = nullptr;

};

} // end namespace gui
} // end namespace mdn
