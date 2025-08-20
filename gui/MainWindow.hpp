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

#include "DigitGridWidget.hpp"

#include "../library/GlobalConfig.hpp"


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

private:
    void createMenus();
    void setupLayout();

    QSplitter* m_splitter = nullptr;

    // MDN Digit Browser (upper pane)
    QTabWidget* m_tabWidget = nullptr;

    // Command History (lower pane)
    QTextEdit* m_commandHistory = nullptr;
    QLineEdit* m_commandInput = nullptr;
    QPushButton* m_submitButton = nullptr;
    QPushButton* m_copyButton = nullptr;

    QMap<int, mdn::Mdn2d*> m_mdnMap;
    int m_nextMdnId = 0;

};
