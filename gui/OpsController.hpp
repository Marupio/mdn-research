#pragma once

#include <ostream>

#include <QObject>
#include <QStringList>

#include "EnumDestinationMode.hpp"
#include "EnumOperation.hpp"
#include "MdnQtInterface.hpp"
#include "OperationStrip.hpp"

class QMainWindow;
class QMenu;
class QMenuBar;
class QSplitter;
class QTabWidget;
class QWidget;

// // Forward declarations
// namespace mdn {
// namespace gui {
// class BinaryOperationDialog;
// } // end namespace gui
// } // end namespace mdn

namespace mdn {
namespace gui {

class OpsController : public QObject {
    Q_OBJECT

public:

    struct Plan {
        Operation op;
        int indexA;
        int indexB;
        DestinationSimple dest;
        int overwriteIndex; // -1 means “not set”
        QString newName;

        friend std::ostream& operator<<(std::ostream& os, const Plan& p) {
            std::string destStr(
                p.dest == DestinationSimple::InPlace
                    ? "InPlace"
                    : "ToNew(" + MdnQtInterface::fromQString(p.newName) + ")"
            );
            os << "[" << p.indexA << OperationToOpStr(p.op) << p.indexB
                << "→" << destStr << ",o:" << p.overwriteIndex << "]";
            return os;
        }
    };

public:
    OpsController(QMainWindow* mw, QTabWidget* tabs, QWidget* history, QObject* parent = nullptr);

    QWidget* bottomContainer() const;

signals:
    void planReady(const OpsController::Plan& plan);

public slots:
    void battleStations(Operation op);
    void refreshTabNames();

private slots:
    void battleStations();
    void onMenuAdd();
    void onMenuSub();
    void onMenuMul();
    void onMenuDiv();

    void onStripRequest(
        Operation op,
        int indexA,
        int indexB,
        DestinationSimple dest
    );
    void onStripChangeB();

private:
    void buildMenus();
    void rebuildBottomContainer();
    QStringList collectTabNames() const;
    int activeIndex() const;
    void runDialog(Operation preset);
    void runQuick(Operation op, DestinationSimple dest);
    DestinationSimple stripDestToController(DestinationSimple d) const;
    Operation stripOpToController(Operation o) const;

private:
    QMainWindow* m_mainWindow{nullptr};
    QTabWidget* m_tabs{nullptr};
    QWidget* m_history{nullptr};

    QWidget* m_bottomContainer{nullptr};
    OperationStrip* m_strip{nullptr};

    QMenu* m_menuOps{nullptr};

    int m_rememberedB{0};
    DestinationSimple m_rememberedDest{DestinationSimple::InPlace};
};

} // end namespace gui
} // end namespace mdn
