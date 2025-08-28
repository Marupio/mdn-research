#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

#include "CommandWidget.hpp"
#include "OpsController.hpp"
#include "Project.hpp"

#include "../library/Mdn2d.hpp"

namespace mdn {
namespace gui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

signals:
    void newProjectRequested();
    void newMdn2dRequested();
    void openProjectRequested();
    void openMdn2dRequested();
    void saveProjectRequested();
    void saveMdn2dRequested();
    void closeProjectRequested();

private slots:
    void onTabContextMenu(const QPoint& pos);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void createSplitter();
    void createMenus();
    void setupLayout();
    void createNewProject();
    void createTabs();
    void createTabForIndex(int index);
    void initOperationsUi();

    void onTabMoved(int from, int to);
    void onTabCloseRequested(int index);
    void renameTab(int index);
    void duplicateTab(int index);
    void copyTab(int index);
    void pasteTab(int insertAt);
    void syncTabsToProject();

    // Focus model API
    void initFocusModel();
    void focusActiveGrid();
    QWidget* activeGridWidget() const;
    bool isFocusAllowedWidget(QWidget* w) const;
    void onAppFocusChanged(QWidget* old, QWidget* now);

    QSplitter* m_splitter = nullptr;
    double m_splitRatio{0.5};
    void applySplitRatio();

    // MDN Digit Browser (upper pane)
    QTabWidget* m_tabWidget = nullptr;

    // Command History (lower pane)
    CommandWidget* m_command = nullptr;

    OpsController* m_ops{nullptr};
    Project* m_project = nullptr;

private slots:
    void onSplitterMoved(int pos, int index);
    void onCommandSubmitted(const QString& text);
    void onOpsPlan(const OpsController::Plan& p);

    void slotSelectNextTab();
    void slotSelectPrevTab();
    void slotMoveTabRight();
    void slotMoveTabLeft();
    void slotDebugShowAllTabs();

};

} // end namespace gui
} // end namespace mdn
