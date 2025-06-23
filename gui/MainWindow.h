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

#include "../library/PlaceHolderMdn.h"

class MainWindow : public QMainWindow {
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
    QTabWidget* tabWidget;
    QMap<int, mdn::PlaceHolderMdn*> mdnMap;
    int nextMDNId = 0;

    void createMDNTab(int id);
};

#endif // MAINWINDOW_H
