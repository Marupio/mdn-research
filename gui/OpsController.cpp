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
#include "OperationStrip.hpp"


mdn::gui::OpsController::OpsController(
    QMainWindow* mw,
    QTabWidget* tabs,
    QWidget* history,
    QObject* parent
) :
    QObject(parent), m_mainWindow(mw), m_tabs(tabs), m_history(history)
{
    Log_Debug3_H("");
    buildMenus();
    rebuildBottomContainer();
    refreshTabNames();
    Log_Debug3_T("");
}


QWidget* mdn::gui::OpsController::bottomContainer() const {
    return m_bottomContainer;
}


void mdn::gui::OpsController::refreshTabNames() {
    Log_Debug3_H("");
    QStringList names = collectTabNames();
    int a = activeIndex();
    if (m_strip) {
        m_strip->setTabNames(names);
        m_strip->setActiveIndex(a);
        m_strip->setRememberedB(m_rememberedB);
        m_strip->setDestinationMode(
            m_rememberedDest == DestinationSimple::InPlace
            ? DestinationSimple::InPlace
            : DestinationSimple::ToNew
        );
    }
    Log_Debug3_T("");
}


void mdn::gui::OpsController::onMenuAdd() {
    Log_Debug3_H("");
    runDialog(Operation::Add);
    Log_Debug3_T("");
}


void mdn::gui::OpsController::onMenuSub() {
    Log_Debug3_H("");
    runDialog(Operation::Subtract);
    Log_Debug3_T("");
}


void mdn::gui::OpsController::onMenuMul() {
    Log_Debug3_H("");
    runDialog(Operation::Multiply);
    Log_Debug3_T("");
}


void mdn::gui::OpsController::onMenuDiv() {
    Log_Debug3_H("");
    runDialog(Operation::Divide);
    Log_Debug3_T("");
}


void mdn::gui::OpsController::onStripRequest(
    Operation op,
    int indexA,
    int indexB,
    DestinationSimple dest
) {
    Log_Debug3_H(
        OperationToString(op) << "(" << indexA << "," << indexB << ")"
    );
    OpsController::Plan p;
    p.op = stripOpToController(op);
    p.indexA = indexA;
    p.indexB = indexB;
    p.dest = stripDestToController(dest);
    if (p.dest == DestinationSimple::ToNew) {
        QStringList names = collectTabNames();
        QString a = names.value(p.indexA);
        QString b = names.value(p.indexB);
        p.newName = a + " " + OperationToOpQStr(p.op) + " " + b;
    }
    m_rememberedB = indexB;
    m_rememberedDest = stripDestToController(dest);
    Log_Debug3_T("emitting " << p);
    emit planReady(p);
}


void mdn::gui::OpsController::onStripChangeB() {
    runDialog(Operation::Add);
}


void mdn::gui::OpsController::buildMenus() {
    QMenuBar* mb = m_mainWindow->menuBar();
    QList<QAction*> menus = mb->actions();

    QMenu* toolsMenu = nullptr;
    QMenu* helpMenu = nullptr;

    for (QAction* act : menus) {
        QMenu* m = act->menu();
        if (!m) {
            continue;
        }
        if (m->title().toLower().contains("tools")) {
            toolsMenu = m;
        } else {
            if (m->title().toLower().contains("help")) {
                helpMenu = m;
            }
        }
    }

    int insertPos = mb->actions().size();
    if (helpMenu) {
        insertPos = mb->actions().indexOf(helpMenu->menuAction());
    }

    m_menuOps = new QMenu(tr("Operations"), mb);
    QAction* opsAction = m_menuOps->menuAction();
    mb->insertAction(insertPos >= 0 ? mb->actions().value(insertPos) : nullptr, opsAction);

    QAction* actAdd = new QAction(tr("Add…"), m_menuOps);
    QAction* actSub = new QAction(tr("Subtract…"), m_menuOps);
    QAction* actMul = new QAction(tr("Multiply…"), m_menuOps);
    QAction* actDiv = new QAction(tr("Divide…"), m_menuOps);

    connect(actAdd, SIGNAL(triggered()), this, SLOT(onMenuAdd()));
    connect(actSub, SIGNAL(triggered()), this, SLOT(onMenuSub()));
    connect(actMul, SIGNAL(triggered()), this, SLOT(onMenuMul()));
    connect(actDiv, SIGNAL(triggered()), this, SLOT(onMenuDiv()));

    QAction* sep = new QAction(m_menuOps);
    sep->setSeparator(true);

    m_menuOps->addAction(actAdd);
    m_menuOps->addAction(actSub);
    m_menuOps->addAction(actMul);
    m_menuOps->addAction(actDiv);
}


void mdn::gui::OpsController::rebuildBottomContainer() {
    if (!m_history) {
        return;
    }
    QWidget* parent = m_history->parentWidget();
    m_bottomContainer = new QWidget(parent);

    QVBoxLayout* lay = new QVBoxLayout(m_bottomContainer);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    m_strip = new OperationStrip(m_bottomContainer);
    lay->addWidget(m_strip);

    m_history->setParent(m_bottomContainer);
    lay->addWidget(m_history, 1);

    connect(
        m_strip,
        SIGNAL(
            requestOperation(
                Operation,
                int,
                int,
                DestinationSimple
            )
        ),
        this,
        SLOT(
            onStripRequest(
                Operation,
                int,
                int,
                DestinationSimple
            )
        )
    );
    connect(m_strip, SIGNAL(requestChangeB()), this, SLOT(onStripChangeB()));
}


QStringList mdn::gui::OpsController::collectTabNames() const {
    QStringList out;
    if (!m_tabs) {
        return out;
    }
    int skipPlusTab = m_tabs->count()-1;
    for (int i = 0; i < skipPlusTab; ++i) {
        out << m_tabs->tabText(i);
    }
    return out;
}


int mdn::gui::OpsController::activeIndex() const {
    if (!m_tabs) {
        return 0;
    }
    int idx = m_tabs->currentIndex();
    if (idx < 0) {
        idx = 0;
    }
    return idx;
}


void mdn::gui::OpsController::runDialog(Operation preset) {
    Log_Debug3_H("" << preset);
Log_Debug4("");
    QStringList names = collectTabNames();
Log_Debug4("");
    if (names.size() < 2) {
        Log_Debug3_T("names.size() = " << names.size());
        return;
    }
Log_Debug4("");
    BinaryOperationDialog dlg(m_mainWindow);

Log_Debug4("");
    dlg.setTabNames(names);
Log_Debug4("");
    dlg.setActiveIndex(activeIndex());
Log_Debug4("");

    dlg.setInitialOperation(preset);
Log_Debug4("");

    int b = m_rememberedB;
Log_Debug4("");
    if (b < 0 || b >= names.size()) {
        b = 0;
    }
Log_Debug4("");
    dlg.setRememberedB(b);
Log_Debug4("");

    DestinationMode d = DestinationMode::OverwriteA;
Log_Debug4("");
    if (m_rememberedDest == DestinationSimple::InPlace) {
        d = DestinationMode::OverwriteA;
    } else {
        d = DestinationMode::CreateNew;
    }
Log_Debug4("");
    dlg.setRememberedDestination(d);
Log_Debug4("");

    int res = dlg.exec();
    if (res != QDialog::Accepted) {
        Log_Debug3_T("Not accepted");
        return;
    }

Log_Debug4("");
    BinaryOperationDialog::Plan pp = dlg.plan();
Log_Debug4("");
    Log_Debug4("pp=" << pp);

    OpsController::Plan p;
Log_Debug4("");
    p.op = preset;
Log_Debug4("");
    p.indexA = pp.indexA;
Log_Debug4("");
    p.indexB = pp.indexB;
Log_Debug4("");
    p.overwriteIndex = -1;
Log_Debug4("");
    if (pp.dest == DestinationMode::CreateNew) {
        p.dest = DestinationSimple::ToNew;
Log_Debug4("");
        p.newName = pp.newName;
Log_Debug4("");
    } else {
        p.dest = DestinationSimple::InPlace;
Log_Debug4("");
        p.overwriteIndex = pp.indexDest;
Log_Debug4("");
        if (p.overwriteIndex == pp.indexB && pp.indexB != pp.indexA) {
            p.indexA = pp.indexB;
Log_Debug4("");
            p.indexB = pp.indexA;
Log_Debug4("");
        }
    }

Log_Debug4("");
    if (pp.rememberChoices) {
        m_rememberedB = pp.indexB;
Log_Debug4("");
        if (p.dest == DestinationSimple::InPlace) {
            m_rememberedDest = DestinationSimple::InPlace;
Log_Debug4("");
        } else {
            m_rememberedDest = DestinationSimple::ToNew;
Log_Debug4("");
        }
    }

    Log_Debug3_T("Emitting " << p);
    emit planReady(p);
}


void mdn::gui::OpsController::runQuick(Operation op, DestinationSimple dest) {
    Log_Debug3_H("op=" << op << ", dest=" << dest);
    QStringList names = collectTabNames();
    if (names.size() < 2) {
        runDialog(op);
        Log_Debug3_T("names.size() = " << names.size());
        return;
    }
    int a = activeIndex();
    int b = m_rememberedB;
    if (b < 0 || b >= names.size()) {
        b = (a == 0 ? 1 : 0);
    }

    OpsController::Plan p;
    p.op = op;
    p.indexA = a;
    p.indexB = b;
    p.dest = dest;
    if (dest == DestinationSimple::ToNew) {
        QString sym = "+";
        if (op == Operation::Subtract) {
            sym = "-";
        } else {
            if (op == Operation::Multiply) {
                sym = "×";
            } else {
                sym = "÷";
            }
        }
        p.newName = names.value(a) + " " + sym + " " + names.value(b);
    }
    Log_Debug3_T("Emitting " << p);
    emit planReady(p);
}


mdn::gui::DestinationSimple mdn::gui::OpsController::stripDestToController(
    DestinationSimple d
) const {
    if (d == DestinationSimple::InPlace) {
        return DestinationSimple::InPlace;
    } else {
        return DestinationSimple::ToNew;
    }
}


mdn::gui::Operation mdn::gui::OpsController::stripOpToController(
    Operation o
) const {
    if (o == Operation::Add) {
        return Operation::Add;
    } else {
        if (o == Operation::Subtract) {
            return Operation::Subtract;
        } else {
            if (o == Operation::Multiply) {
                return Operation::Multiply;
            } else {
                return Operation::Divide;
            }
        }
    }
}
