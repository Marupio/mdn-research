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

#include "Project.hpp"

// #include "DigitGridWidget.hpp"

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

    void onTabMoved(int from, int to);
    void onTabCloseRequested(int index);
    void renameTab(int index);
    void duplicateTab(int index);
    void copyTab(int index);
    void pasteTab(int insertAt);
    void syncTabsToProject();

    QSplitter* m_splitter = nullptr;
    double m_splitRatio{0.5};
    void applySplitRatio();

    // MDN Digit Browser (upper pane)
    QTabWidget* m_tabWidget = nullptr;

    // Command History (lower pane)
    QTextEdit* m_commandHistory = nullptr;
    QLineEdit* m_commandInput = nullptr;
    QPushButton* m_submitButton = nullptr;
    QPushButton* m_copyButton = nullptr;

    Project* m_project = nullptr;

private slots:
    void onSplitterMoved(int pos, int index);

};

} // end namespace gui
} // end namespace mdn
