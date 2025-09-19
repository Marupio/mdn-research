#include "OpsController.hpp"

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>

#include "../library/Logger.hpp"
#include "BinaryOperationDialog.hpp"
#include "CommandWidget.hpp"
#include "HoverPeekTabWidget.hpp"
#include "OperationStrip.hpp"
#include "Project.hpp"


mdn::gui::OpsController::OpsController(
    QMainWindow* mw,
    Project* project,
    HoverPeekTabWidget* tabs,
    CommandWidget* command,
    // QWidget* history,
    QObject* parent
) :
    QObject(parent),
    m_mainWindow(mw),
    m_project(project),
    m_tabs(tabs),
    m_command(command)
{
    Log_Debug3_H("");
    // buildMenus();
    rebuildBottomContainer();

    // choose B / destination via tab clicks
    if (m_tabs) {
        connect(
            m_tabs,
            &HoverPeekTabWidget::committedIndex,
            this,
            &OpsController::onTabCommitted
        );
    }
    Log_Debug3_T("");
}


QWidget* mdn::gui::OpsController::bottomContainer() const {
    return m_bottomContainer;
}


bool mdn::gui::OpsController::isActive() const {
    return m_phase != OperationPhase::Idle;
}


void mdn::gui::OpsController::resetModel(Project* project) {
    m_project = project;
}


void mdn::gui::OpsController::clearModel() {
    m_project = nullptr;
}


void mdn::gui::OpsController::onTabCommitted(int idx) {
    if (!m_project || !m_tabs || m_phase == OperationPhase::Idle) return;

    if (m_phase == OperationPhase::PickB) {
        m_b = idx;
        m_phase = OperationPhase::PickDest;
        emit requestStatus(QString("%1 %2 %3  =  *Choose*")
            .arg(nameFor(m_a))
            .arg(QString::fromStdString(OperationToOpStr(m_op)))
            .arg(nameFor(m_b)), 0
        );
        return;
    }

    if (m_phase == OperationPhase::PickDest) {
        finishTo(idx, false);
    }
}


void mdn::gui::OpsController::onStripOperation(Operation op) {
    begin(op);
}


void mdn::gui::OpsController::onStripCancel() {
    cancel();
}


void mdn::gui::OpsController::onStripNewTab() {
    if (m_phase != OperationPhase::PickDest) return;

    // QString defaultName(
    //     QString("%1%2%3")
    //         .arg(nameFor(m_a))
    //         .arg(QString::fromStdString(OperationToOpStr(m_op)))
    //         .arg(nameFor(m_b)
    //     )
    // );
    // // Ask MainWindow to create & select a new tab
    // QMetaObject::invokeMethod(parent(), "onNewNamedMdn2d", Qt::QueuedConnection, defaultName);
    // const int dest = m_tabs ? m_tabs->currentIndex() : -1;

    // After creation, assume selection moved to the new tab:
    finishTo(-1, true);
}


// void mdn::gui::OpsController::onMenuAdd() {
//     Log_Debug3_H("");
//     runDialog(Operation::Add);
//     Log_Debug3_T("");
// }
//
//
// void mdn::gui::OpsController::onMenuSub() {
//     Log_Debug3_H("");
//     runDialog(Operation::Subtract);
//     Log_Debug3_T("");
// }
//
//
// void mdn::gui::OpsController::onMenuMul() {
//     Log_Debug3_H("");
//     runDialog(Operation::Multiply);
//     Log_Debug3_T("");
// }
//
//
// void mdn::gui::OpsController::onMenuDiv() {
//     Log_Debug3_H("");
//     runDialog(Operation::Divide);
//     Log_Debug3_T("");
// }
//
//
// void mdn::gui::OpsController::onStripRequest(
//     Operation op,
//     int indexA,
//     int indexB,
//     DestinationSimple dest
// ) {
//     Log_Debug3_H(
//         OperationToString(op) << "(" << indexA << "," << indexB << ")"
//     );
//     OpsController::Plan p;
//     p.op = stripOpToController(op);
//     p.indexA = indexA;
//     p.indexB = indexB;
//     p.dest = stripDestToController(dest);
//     if (p.dest == DestinationSimple::ToNew) {
//         QStringList names = collectTabNames();
//         QString a = names.value(p.indexA);
//         QString b = names.value(p.indexB);
//         p.newName = a + " " + OperationToOpQStr(p.op) + " " + b;
//     }
//     m_rememberedB = indexB;
//     m_rememberedDest = stripDestToController(dest);
//     Log_Debug3_T("emitting " << p);
//     emit planReady(p);
// }


void mdn::gui::OpsController::rebuildBottomContainer() {
    m_bottomContainer = new QWidget;
    m_bottomLayout = new QVBoxLayout(m_bottomContainer);
    m_bottomLayout->setContentsMargins(0,0,0,0);
    m_bottomLayout->setSpacing(6);

    m_strip = new OperationStrip(m_bottomContainer);
    m_bottomLayout->addWidget(m_strip);

    if (m_command) {
        m_command->setParent(m_bottomContainer);
        m_bottomLayout->addWidget(m_command);
    }

    connect(
        m_strip,
        &OperationStrip::requestOperation,
        this,
        &OpsController::onStripOperation
    );
    connect(
        m_strip,
        &OperationStrip::requestCancel,
        this,
        &OpsController::onStripCancel
    );
    connect(
        m_strip,
        &OperationStrip::requestNewTab,
        this,
        &OpsController::onStripNewTab
    );
}


void mdn::gui::OpsController::begin(Operation op) {
    if (!m_project || !m_tabs) return;
    m_op = op;
    m_a = m_tabs->currentIndex(); // A = current tab
    m_b = -1;
    m_phase = OperationPhase::PickB;
    if (m_strip) m_strip->setCancelEnabled(true);
    emit requestStatus(QString("%1 %2  *Choose*")
        .arg(nameFor(m_a))
        .arg(QString::fromStdString(OperationToOpStr(m_op))), 0);
}


void mdn::gui::OpsController::finishTo(int destIndex, bool isNew) {
    if (m_a < 0 || m_b < 0 || destIndex < 0) {
        cancel();
        return;
    }

    emit requestStatus(QString("%1 %2 %3  →  %4")
        .arg(nameFor(m_a))
        .arg(QString::fromStdString(OperationToOpStr(m_op)))
        .arg(nameFor(m_b))
        .arg(nameFor(destIndex)), 0);


    QString defaultName = "";
    if (destIndex < 0) {
        defaultName = QString(
            QString("%1%2%3")
                .arg(nameFor(m_a))
                .arg(QString::fromStdString(OperationToOpStr(m_op)))
                .arg(nameFor(m_b)
            )
        );
    }


    Plan p;
    p.op = m_op;
    p.indexA = m_a;
    p.indexB = m_b;
    p.indexDest = destIndex;
    p.newName = defaultName;

    emit planReady(p);

    m_phase = OperationPhase::Idle; m_a = m_b = -1;
    if (m_strip) m_strip->setCancelEnabled(false);
}


void mdn::gui::OpsController::cancel() {
    m_phase = OperationPhase::Idle; m_a = m_b = -1;
    if (m_strip) m_strip->setCancelEnabled(false);
    emit requestStatus(QString(), 0);
}


QString mdn::gui::OpsController::nameFor(int tabIndex) const {
    if (!m_project) {
        return {};
    }
    return QString::fromStdString(m_project->nameOfMdn(tabIndex));
}


// void mdn::gui::OpsController::buildMenus() {
//     QMenuBar* mb = m_mainWindow->menuBar();
//     QList<QAction*> menus = mb->actions();
//
//     QMenu* toolsMenu = nullptr;
//     QMenu* helpMenu = nullptr;
//
//     for (QAction* act : menus) {
//         QMenu* m = act->menu();
//         if (!m) {
//             continue;
//         }
//         if (m->title().toLower().contains("tools")) {
//             toolsMenu = m;
//         } else {
//             if (m->title().toLower().contains("help")) {
//                 helpMenu = m;
//             }
//         }
//     }
//
//     int insertPos = mb->actions().size();
//     if (helpMenu) {
//         insertPos = mb->actions().indexOf(helpMenu->menuAction());
//     }
//
//     m_menuOps = new QMenu(tr("Operations"), mb);
//     QAction* opsAction = m_menuOps->menuAction();
//     mb->insertAction(insertPos >= 0 ? mb->actions().value(insertPos) : nullptr, opsAction);
//
//     QAction* actAdd = new QAction(tr("Add…"), m_menuOps);
//     QAction* actSub = new QAction(tr("Subtract…"), m_menuOps);
//     QAction* actMul = new QAction(tr("Multiply…"), m_menuOps);
//     QAction* actDiv = new QAction(tr("Divide…"), m_menuOps);
//
//     connect(actAdd, SIGNAL(triggered()), this, SLOT(onMenuAdd()));
//     connect(actSub, SIGNAL(triggered()), this, SLOT(onMenuSub()));
//     connect(actMul, SIGNAL(triggered()), this, SLOT(onMenuMul()));
//     connect(actDiv, SIGNAL(triggered()), this, SLOT(onMenuDiv()));
//
//     QAction* sep = new QAction(m_menuOps);
//     sep->setSeparator(true);
//
//     QAction* actAddIn = new QAction(tr("Add (in place)"), m_menuOps);
//     QAction* actAddNew = new QAction(tr("Add (to new)"), m_menuOps);
//     QAction* actSubIn = new QAction(tr("Subtract (in place)"), m_menuOps);
//     QAction* actSubNew = new QAction(tr("Subtract (to new)"), m_menuOps);
//     QAction* actMulIn = new QAction(tr("Multiply (in place)"), m_menuOps);
//     QAction* actMulNew = new QAction(tr("Multiply (to new)"), m_menuOps);
//     QAction* actDivIn = new QAction(tr("Divide (in place)"), m_menuOps);
//     QAction* actDivNew = new QAction(tr("Divide (to new)"), m_menuOps);
//
//     connect(actAddIn, SIGNAL(triggered()), this, SLOT(onMenuAddInPlace()));
//     connect(actAddNew, SIGNAL(triggered()), this, SLOT(onMenuAddToNew()));
//     connect(actSubIn, SIGNAL(triggered()), this, SLOT(onMenuSubInPlace()));
//     connect(actSubNew, SIGNAL(triggered()), this, SLOT(onMenuSubToNew()));
//     connect(actMulIn, SIGNAL(triggered()), this, SLOT(onMenuMulInPlace()));
//     connect(actMulNew, SIGNAL(triggered()), this, SLOT(onMenuMulToNew()));
//     connect(actDivIn, SIGNAL(triggered()), this, SLOT(onMenuDivInPlace()));
//     connect(actDivNew, SIGNAL(triggered()), this, SLOT(onMenuDivToNew()));
//
//     m_menuOps->addAction(actAdd);
//     m_menuOps->addAction(actSub);
//     m_menuOps->addAction(actMul);
//     m_menuOps->addAction(actDiv);
//     m_menuOps->addAction(sep);
//     m_menuOps->addAction(actAddIn);
//     m_menuOps->addAction(actAddNew);
//     m_menuOps->addAction(actSubIn);
//     m_menuOps->addAction(actSubNew);
//     m_menuOps->addAction(actMulIn);
//     m_menuOps->addAction(actMulNew);
//     m_menuOps->addAction(actDivIn);
//     m_menuOps->addAction(actDivNew);
// }


// void mdn::gui::OpsController::runDialog(Operation preset) {
//     Log_Debug3_H("" << preset);
//     QStringList names = collectTabNames();
//     if (names.size() < 2) {
//         Log_Debug3_T("names.size() = " << names.size());
//         return;
//     }
//     BinaryOperationDialog dlg(m_mainWindow);
//
//     dlg.setTabNames(names);
//     dlg.setActiveIndex(activeIndex());
//
//     dlg.setInitialOperation(preset);
//
//     int b = m_rememberedB;
//     if (b < 0 || b >= names.size()) {
//         b = 0;
//     }
//     dlg.setRememberedB(b);
//
//     DestinationMode d = DestinationMode::OverwriteA;
//     if (m_rememberedDest == DestinationSimple::InPlace) {
//         d = DestinationMode::OverwriteA;
//     } else {
//         d = DestinationMode::CreateNew;
//     }
//     dlg.setRememberedDestination(d);
//
//     int res = dlg.exec();
//     if (res != QDialog::Accepted) {
//         Log_Debug3_T("Not accepted");
//         return;
//     }
//
//     BinaryOperationDialog::Plan pp = dlg.plan();
//     Log_Debug4("pp=" << pp);
//
//     OpsController::Plan p;
//     p.op = preset;
//     p.indexA = pp.indexA;
//     p.indexB = pp.indexB;
//     p.overwriteIndex = -1;
//     if (pp.dest == DestinationMode::CreateNew) {
//         p.dest = DestinationSimple::ToNew;
//         p.newName = pp.newName;
//     } else {
//         p.dest = DestinationSimple::InPlace;
//         p.overwriteIndex = pp.indexDest;
//         if (p.overwriteIndex == pp.indexB && pp.indexB != pp.indexA) {
//             p.indexA = pp.indexB;
//             p.indexB = pp.indexA;
//         }
//     }
//
//     if (pp.rememberChoices) {
//         m_rememberedB = pp.indexB;
//         if (p.dest == DestinationSimple::InPlace) {
//             m_rememberedDest = DestinationSimple::InPlace;
//         } else {
//             m_rememberedDest = DestinationSimple::ToNew;
//         }
//     }
//
//     Log_Debug3_T("Emitting " << p);
//     emit planReady(p);
// }


// void mdn::gui::OpsController::runQuick(Operation op, DestinationSimple dest) {
//     Log_Debug3_H("op=" << op << ", dest=" << dest);
//     QStringList names = collectTabNames();
//     if (names.size() < 2) {
//         runDialog(op);
//         Log_Debug3_T("names.size() = " << names.size());
//         return;
//     }
//     int a = activeIndex();
//     int b = m_rememberedB;
//     if (b < 0 || b >= names.size()) {
//         b = (a == 0 ? 1 : 0);
//     }
//
//     OpsController::Plan p;
//     p.op = op;
//     p.indexA = a;
//     p.indexB = b;
//     p.dest = dest;
//     if (dest == DestinationSimple::ToNew) {
//         QString sym = "+";
//         if (op == Operation::Subtract) {
//             sym = "-";
//         } else {
//             if (op == Operation::Multiply) {
//                 sym = "×";
//             } else {
//                 sym = "÷";
//             }
//         }
//         p.newName = names.value(a) + " " + sym + " " + names.value(b);
//     }
//     Log_Debug3_T("Emitting " << p);
//     emit planReady(p);
// }
