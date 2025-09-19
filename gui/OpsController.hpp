#pragma once

#include <ostream>

#include <QObject>
#include <QVBoxLayout>
#include <QWidget>

#include "EnumDestinationMode.hpp"
#include "EnumOperation.hpp"
#include "EnumOperationPhase.hpp"
#include "HoverPeekTabWidget.hpp"
#include "MdnQtInterface.hpp"
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
class OperationStrip;
class Project;

} // end namespace gui
} // end namespace mdn


namespace mdn {
namespace gui {

class OpsController : public QObject {
    Q_OBJECT

public:

    struct Plan {
        Operation op;
        int indexA;
        int indexB;
        // indexDest: when overwriting an existing tab, contains tab index, -1 if creating new
        int indexDest;
        // newName: when writing to a new tab, this is the name to try
        QString newName;
        // DestinationSimple dest;
        // int overwriteIndex; // -1 means “not set”
        // QString newName;

        friend std::ostream& operator<<(std::ostream& os, const Plan& p) {
            std::string destStr(
                p.indexDest < 0
                    ? "ToNew(" + MdnQtInterface::fromQString(p.newName) + ")"
                    : "Overwrite(" + std::to_string(p.indexDest) + ")"
            );
            os << "[" << p.indexA << OperationToOpStr(p.op) << p.indexB
                << "→" << destStr << "]";
            return os;
        }
    };

public:
    OpsController(
        QMainWindow* mw,
        Project* project,
        HoverPeekTabWidget* tabs,
        CommandWidget* command,
        QObject* parent = nullptr
    );

    QWidget* bottomContainer() const;
    bool isActive() const;
    void resetModel(Project* project);
    void clearModel();


signals:
    // to status bar
    void requestStatus(const QString& s, int timeOut);
    // We've decided on a plan, over to you, MainWindow
    void planReady(const OpsController::Plan& plan);

public slots:
    // from HoverPeekTabWidget
    void onTabCommitted(int idx);
    void onStripOperation(Operation op);
    void onStripCancel();
    void onStripNewTab();

private slots:
    // void onMenuAdd();
    // void onMenuSub();
    // void onMenuMul();
    // void onMenuDiv();

    // void onStripRequest(
    //     Operation op,
    //     int indexA,
    //     int indexB,
    //     DestinationSimple dest
    // );

private:

    // builds strip + command/history
    void rebuildBottomContainer();
    // start op: A = current
    void begin(Operation op);
    void finishTo(int destIndex, bool isNew);
    void cancel();
    QString nameFor(int tabIndex) const;
    // void buildMenus();
    // void runDialog(Operation preset);
    // void runQuick(Operation op, DestinationSimple dest);

    QMainWindow* m_mainWindow{nullptr};
    mdn::gui::Project* m_project = nullptr;
    mdn::gui::HoverPeekTabWidget* m_tabs = nullptr;
    CommandWidget* m_command = nullptr;

    QWidget* m_bottomContainer = nullptr;
    QVBoxLayout* m_bottomLayout = nullptr;
    OperationStrip* m_strip = nullptr;

    OperationPhase m_phase { OperationPhase::Idle };
    Operation m_op { Operation::Add };
    int m_a { -1 }, m_b { -1 };

    // QWidget* m_history{nullptr};
    // QMenu* m_menuOps{nullptr};
};

} // end namespace gui
} // end namespace mdn
