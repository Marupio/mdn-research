#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QMap>

#include "../library/MultiDimensionalNumber.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addNewMDN();
    void addValueToCurrent();

private:
    QTabWidget* tabWidget;
    QMap<int, MultiDimensionalNumber*> mdnMap;
    int nextMDNId = 0;

    void createMDNTab(int id);
};

#endif // MAINWINDOW_H
