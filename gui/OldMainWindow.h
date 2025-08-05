#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QMap>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QMouseEvent>

#include <QLabel>

#include "../library/GlobalConfig.h"
#include "../library/Mdn2d.h"

class MDN_API MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addNewMDN();
    void addValueToCurrent();
    void renameTab();
    void duplicateTab();
    void deleteTab();
    void onTabBarDoubleClicked(int index);
    void onTabContextMenuRequested(const QPoint &pos);

private:
    QTabWidget* m_tabWidget;
    QMap<int, mdn::Mdn2d*> m_mdnMap;
    int m_nextMdnId = 0;

    void createMDNTab(int id);
};

#endif // MAINWINDOW_H
