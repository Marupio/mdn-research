//////// Project.cpp
// Example emit points you can add where the model actually mutates tabs

void mdn::gui::Project::insertMdn(Mdn2d& mdn, int index)
{
    Q_EMIT tabsAboutToChange();

    // existing insert logic, compute the effective insertion index `index`

    const int newCurrent = index;
    Q_EMIT tabsChanged(newCurrent);
}

bool mdn::gui::Project::deleteMdn(int index)
{
    Q_EMIT tabsAboutToChange();

    // existing delete logic; return false early if invalid index

    const int newCount = static_cast<int>(m_data.size());
    int newCurrent = index;
    {
        if (newCurrent >= newCount) {
            newCurrent = newCount - 1;
        }
        if (newCurrent < 0) {
            newCurrent = -1;
        }
    }

    Q_EMIT tabsChanged(newCurrent);
    return true;
}

//////// MainWindow.hpp

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
#include "Project.hpp"

namespace mdn {
namespace gui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

private slots:
    void onProjectTabsAboutToChange();
    void onProjectTabsChanged(int currentIndex);

private:
    QSplitter* m_splitter{nullptr};
    QTabWidget* m_tabWidget{nullptr};
    CommandWidget* m_command{nullptr};
    OpsController* m_ops{nullptr};
    Project* m_project{nullptr};
};

} // namespace gui
} // namespace mdn

//////// MainWindow.cpp
#include "MainWindow.hpp"
#include <QEvent>
#include <QInputDialog>
#include <QList>
#include <QResizeEvent>
#include <QSplitter>
#include <QTabBar>
#include <QTabWidget>
#include "../library/Logger.hpp"

mdn::gui::MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // your existing setup
    // wherever you create or assign m_project, add the connects

    // Example if you create a new project:
    // m_project = new Project(this);
    if (m_project) {
        connect(m_project, &mdn::gui::Project::tabsAboutToChange,
                this, &mdn::gui::MainWindow::onProjectTabsAboutToChange);
        connect(m_project, &mdn::gui::Project::tabsChanged,
                this, &mdn::gui::MainWindow::onProjectTabsChanged);
    }
}

void mdn::gui::MainWindow::onProjectTabsAboutToChange()
{
    Log_Debug2_H("onProjectTabsAboutToChange");
    Log_Debug2_T("");
}

void mdn::gui::MainWindow::onProjectTabsChanged(int currentIndex)
{
    Log_Debug2_H("onProjectTabsChanged currentIndex=" << currentIndex);
    Log_Debug2_T("");
}
