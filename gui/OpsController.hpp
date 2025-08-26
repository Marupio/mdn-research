#pragma once

#include <QObject>
#include <QStringList>

class QMainWindow;
class QMenu;
class QMenuBar;
class QSplitter;
class QTabWidget;
class QWidget;

class OperationStrip;
class BinaryOperationDialog;

class OpsController : public QObject {
    Q_OBJECT

public:
    enum class Op {
        Add,
        Subtract,
        Multiply,
        Divide
    };

    enum class Dest {
        InPlace,
        ToNew
    };

    struct Plan {
        Op op;
        int indexA;
        int indexB;
        Dest dest;
        QString newName;
    };

public:
    OpsController(QMainWindow* mw, QTabWidget* tabs, QWidget* history, QObject* parent = nullptr);

    QWidget* bottomContainer() const;

signals:
    void planReady(const OpsController::Plan& plan);

public slots:
    void refreshTabNames();

private slots:
    void onMenuAdd();
    void onMenuSub();
    void onMenuMul();
    void onMenuDiv();

    void onMenuAddInPlace();
    void onMenuAddToNew();
    void onMenuSubInPlace();
    void onMenuSubToNew();
    void onMenuMulInPlace();
    void onMenuMulToNew();
    void onMenuDivInPlace();
    void onMenuDivToNew();

    void onStripRequest(OperationStrip::Operation op, int indexA, int indexB, OperationStrip::DestinationMode dest);
    void onStripChangeB();

private:
    void buildMenus();
    void rebuildBottomContainer();
    QStringList collectTabNames() const;
    int activeIndex() const;
    void runDialog(Op preset);
    void runQuick(Op op, Dest dest);
    OpsController::Dest stripDestToController(OperationStrip::DestinationMode d) const;
    OpsController::Op stripOpToController(OperationStrip::Operation o) const;

private:
    QMainWindow* m_mainWindow{nullptr};
    QTabWidget* m_tabs{nullptr};
    QWidget* m_history{nullptr};

    QWidget* m_bottomContainer{nullptr};
    OperationStrip* m_strip{nullptr};

    QMenu* m_menuOps{nullptr};

    int m_rememberedB{0};
    Dest m_rememberedDest{Dest::InPlace};
};
