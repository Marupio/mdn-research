#pragma once

#include <ostream>

#include <QObject>
#include <QStringList>

#include "EnumDestinationMode.hpp"
#include "EnumOperation.hpp"
#include "EnumOperationPhase.hpp"
#include "MdnQtInterface.hpp"
#include "OperationPlan.hpp"
#include "OperationStrip.hpp"

class QMainWindow;
class QMenu;
class QMenuBar;
class QSplitter;
class QTabWidget;
class QWidget;

// Forward declarations
namespace mdn {
namespace gui {
class CommandWidget;
class Project;
class HoverPeekTabWidget;
class StatusDisplayWidget;
} // end namespace gui
} // end namespace mdn

namespace mdn {
namespace gui {

class OpsController : public QObject {
    Q_OBJECT

public:

    OpsController(
        QMainWindow* mw,
        Project* project,
        HoverPeekTabWidget* tabs,
        CommandWidget* command,
        QObject* parent = nullptr
    );

    QWidget* bottomContainer() const;
    OperationStrip* strip() { return m_strip; }
    StatusDisplayWidget* status() { return m_status; }

    // Returns true if user is midway through specifying an operation on the bottom strip
    bool inBattle() const;
    void resetModel(Project* project);
    void clearModel();

signals:
    // to status bar
    void requestClearStatus();
    void requestStatus(const QString& s, int timeOut);

    void planReady(const OperationPlan& plan);
    void tabClicked(int idx);
    // relayed from tabwidget, I have to check first if it's "new destination", not just "newMdn2d"
    void plusClicked();


public slots:
    // User has just clicked an operation button in the bottom strip, we are in battle
    void battlestations(Operation op);
    void onCancel();

private slots:
    void onMenuAdd();
    void onMenuSub();
    void onMenuMul();
    void onMenuDiv();

    // I intercept tab signals first to check if we're in battle
    void onTabCommitted(int idx);
    void onPlusClicked();

private:
    void buildMenus();
    void rebuildBottomContainer();
    QStringList collectTabNames() const;
    QString nameFor(int idx) const;
    int activeIndex() const;
    void runDialog(Operation preset);
    void endBattle(int destIndex, bool isNew);
    void cancel();

private:

    QMainWindow* m_mainWindow{nullptr};
    Project* m_project = nullptr;
    HoverPeekTabWidget* m_tabs = nullptr;
    CommandWidget* m_command = nullptr;

    QWidget* m_bottomContainer{nullptr};
    StatusDisplayWidget* m_status{nullptr};
    OperationStrip* m_strip{nullptr};
    OperationPhase m_phase { OperationPhase::Idle };
    Operation m_op { Operation::Add };
    int m_a { -1 }, m_b { -1 };

    QMenu* m_menuOps{nullptr};

    int m_rememberedB{0};
    DestinationSimple m_rememberedDest{DestinationSimple::InPlace};
};

} // end namespace gui
} // end namespace mdn
