#pragma once

#include <ostream>

#include <QObject>
#include <QStringList>

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
class BinaryOperationDialog;
} // end namespace gui
} // end namespace mdn

namespace mdn {
namespace gui {

class OpsController : public QObject {
    Q_OBJECT

public:
    enum class Op {
        Add,
        Subtract,
        Multiply,
        Divide
    };
    static std::string OpToString(const Op& o) {
        switch (o) {
            case Op::Add: {
                return "Add";
            }
            case Op::Subtract: {
                return "Subtract";
            }
            case Op::Multiply: {
                return "Multiply";
            }
            case Op::Divide: {
                return "Divide";
            }
            default: {
                return "Unknown";
            }
        }
    }
    static std::string OpToOpStr(const Op& o) {
        switch (o) {
            case Op::Add: {
                return "+";
            }
            case Op::Subtract: {
                return "-";
            }
            case Op::Multiply: {
                return "×";
            }
            case Op::Divide: {
                return "÷";
            }
            default: {
                return "?";
            }
        }
    }

    enum class Dest {
        InPlace,
        ToNew
    };
    static std::string DestToString(const Dest& d) {
        switch (d) {
            case Dest::InPlace: {
                return "InPlace";
            }
            case Dest::ToNew: {
                return "ToNew";
            }
            default: {
                return "Unknown";
            }
        }
    }

    struct Plan {
        Op op;
        int indexA;
        int indexB;
        Dest dest;
        QString newName;

        friend std::ostream& operator<<(std::ostream& os, const Plan& p) {
            std::string destStr(
                p.dest == Dest::InPlace
                    ? "InPlace"
                    : "ToNew(" + MdnQtInterface::fromQString(p.newName) + ")"
            );
            os << "[" << p.indexA << OpToOpStr(p.op) << p.indexB << "→" << destStr << "]";
            return os;
        }
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

    void onStripRequest(
        OperationStrip::Operation op,
        int indexA,
        int indexB,
        OperationStrip::DestinationMode dest
    );
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

} // end namespace gui
} // end namespace mdn
