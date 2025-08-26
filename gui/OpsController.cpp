#include "OpsController.hpp"
#include "OperationStrip.hpp"
#include "BinaryOperationDialog.hpp"

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>

mdn::gui::OpsController::OpsController(
    QMainWindow* mw,
    QTabWidget* tabs,
    QWidget* history,
    QObject* parent
) :
    QObject(parent), m_mainWindow(mw), m_tabs(tabs), m_history(history)
{
    buildMenus();
    rebuildBottomContainer();
    refreshTabNames();
}


QWidget* mdn::gui::OpsController::bottomContainer() const {
    return m_bottomContainer;
}


void mdn::gui::OpsController::refreshTabNames() {
    QStringList names = collectTabNames();
    int a = activeIndex();
    if (m_strip) {
        m_strip->setTabNames(names);
        m_strip->setActiveIndex(a);
        m_strip->setRememberedB(m_rememberedB);
        m_strip->setDestinationMode(
            m_rememberedDest == Dest::InPlace
            ? OperationStrip::DestinationMode::InPlace
            : OperationStrip::DestinationMode::ToNew
        );
    }
}


void mdn::gui::OpsController::onMenuAdd() {
    runDialog(Op::Add);
}


void mdn::gui::OpsController::onMenuSub() {
    runDialog(Op::Subtract);
}


void mdn::gui::OpsController::onMenuMul() {
    runDialog(Op::Multiply);
}


void mdn::gui::OpsController::onMenuDiv() {
    runDialog(Op::Divide);
}


void mdn::gui::OpsController::onMenuAddInPlace() {
    runQuick(Op::Add, Dest::InPlace);
}


void mdn::gui::OpsController::onMenuAddToNew() {
    runQuick(Op::Add, Dest::ToNew);
}


void mdn::gui::OpsController::onMenuSubInPlace() {
    runQuick(Op::Subtract, Dest::InPlace);
}


void mdn::gui::OpsController::onMenuSubToNew() {
    runQuick(Op::Subtract, Dest::ToNew);
}


void mdn::gui::OpsController::onMenuMulInPlace() {
    runQuick(Op::Multiply, Dest::InPlace);
}


void mdn::gui::OpsController::onMenuMulToNew() {
    runQuick(Op::Multiply, Dest::ToNew);
}


void mdn::gui::OpsController::onMenuDivInPlace() {
    runQuick(Op::Divide, Dest::InPlace);
}


void mdn::gui::OpsController::onMenuDivToNew() {
    runQuick(Op::Divide, Dest::ToNew);
}


void mdn::gui::OpsController::onStripRequest(
    OperationStrip::Operation op,
    int indexA,
    int indexB,
    OperationStrip::DestinationMode dest
) {
    OpsController::Plan p;
    p.op = stripOpToController(op);
    p.indexA = indexA;
    p.indexB = indexB;
    p.dest = stripDestToController(dest);
    if (p.dest == Dest::ToNew) {
        QStringList names = collectTabNames();
        QString sym = "+";
        if (p.op == Op::Subtract) {
            sym = "-";
        } else {
            if (p.op == Op::Multiply) {
                sym = "×";
            } else {
                sym = "÷";
            }
        }
        QString a = names.value(p.indexA);
        QString b = names.value(p.indexB);
        p.newName = a + " " + sym + " " + b;
    }
    m_rememberedB = indexB;
    m_rememberedDest = stripDestToController(dest);
    emit planReady(p);
}


void mdn::gui::OpsController::onStripChangeB() {
    runDialog(Op::Add);
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

    QAction* actAddIn = new QAction(tr("Add (in place)"), m_menuOps);
    QAction* actAddNew = new QAction(tr("Add (to new)"), m_menuOps);
    QAction* actSubIn = new QAction(tr("Subtract (in place)"), m_menuOps);
    QAction* actSubNew = new QAction(tr("Subtract (to new)"), m_menuOps);
    QAction* actMulIn = new QAction(tr("Multiply (in place)"), m_menuOps);
    QAction* actMulNew = new QAction(tr("Multiply (to new)"), m_menuOps);
    QAction* actDivIn = new QAction(tr("Divide (in place)"), m_menuOps);
    QAction* actDivNew = new QAction(tr("Divide (to new)"), m_menuOps);

    connect(actAddIn, SIGNAL(triggered()), this, SLOT(onMenuAddInPlace()));
    connect(actAddNew, SIGNAL(triggered()), this, SLOT(onMenuAddToNew()));
    connect(actSubIn, SIGNAL(triggered()), this, SLOT(onMenuSubInPlace()));
    connect(actSubNew, SIGNAL(triggered()), this, SLOT(onMenuSubToNew()));
    connect(actMulIn, SIGNAL(triggered()), this, SLOT(onMenuMulInPlace()));
    connect(actMulNew, SIGNAL(triggered()), this, SLOT(onMenuMulToNew()));
    connect(actDivIn, SIGNAL(triggered()), this, SLOT(onMenuDivInPlace()));
    connect(actDivNew, SIGNAL(triggered()), this, SLOT(onMenuDivToNew()));

    m_menuOps->addAction(actAdd);
    m_menuOps->addAction(actSub);
    m_menuOps->addAction(actMul);
    m_menuOps->addAction(actDiv);
    m_menuOps->addAction(sep);
    m_menuOps->addAction(actAddIn);
    m_menuOps->addAction(actAddNew);
    m_menuOps->addAction(actSubIn);
    m_menuOps->addAction(actSubNew);
    m_menuOps->addAction(actMulIn);
    m_menuOps->addAction(actMulNew);
    m_menuOps->addAction(actDivIn);
    m_menuOps->addAction(actDivNew);
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
                OperationStrip::Operation,
                int,
                int,
                OperationStrip::DestinationMode
            )
        ),
        this,
        SLOT(
            onStripRequest(
                OperationStrip::Operation,
                int,
                int,
                OperationStrip::DestinationMode
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
    for (int i = 0; i < m_tabs->count(); ++i) {
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


void mdn::gui::OpsController::runDialog(Op preset) {
    QStringList names = collectTabNames();
    if (names.size() < 2) {
        return;
    }
    BinaryOperationDialog dlg(m_mainWindow);

    dlg.setTabNames(names);
    dlg.setActiveIndex(activeIndex());

    BinaryOperationDialog::Operation bop = BinaryOperationDialog::Operation::Add;
    if (preset == Op::Add) {
        bop = BinaryOperationDialog::Operation::Add;
    } else {
        if (preset == Op::Subtract) {
            bop = BinaryOperationDialog::Operation::Subtract;
        } else {
            if (preset == Op::Multiply) {
                bop = BinaryOperationDialog::Operation::Multiply;
            } else {
                bop = BinaryOperationDialog::Operation::Divide;
            }
        }
    }
    dlg.setInitialOperation(bop);

    int b = m_rememberedB;
    if (b < 0 || b >= names.size()) {
        b = 0;
    }
    dlg.setRememberedB(b);

    BinaryOperationDialog::DestinationMode d = BinaryOperationDialog::DestinationMode::OverwriteA;
    if (m_rememberedDest == Dest::InPlace) {
        d = BinaryOperationDialog::DestinationMode::OverwriteA;
    } else {
        d = BinaryOperationDialog::DestinationMode::CreateNew;
    }
    dlg.setRememberedDestination(d);

    int res = dlg.exec();
    if (res != QDialog::Accepted) {
        return;
    }

    BinaryOperationDialog::Plan pp = dlg.plan();

    OpsController::Plan p;
    p.op = preset;
    p.indexA = pp.indexA;
    p.indexB = pp.indexB;
    if (pp.dest == BinaryOperationDialog::DestinationMode::OverwriteA) {
        p.dest = Dest::InPlace;
    } else {
        if (pp.dest == BinaryOperationDialog::DestinationMode::OverwriteB) {
            p.dest = Dest::InPlace;
            p.indexA = pp.indexB;
            p.indexB = pp.indexA;
        } else {
            p.dest = Dest::ToNew;
            p.newName = pp.newName;
        }
    }

    if (pp.rememberChoices) {
        m_rememberedB = pp.indexB;
        if (p.dest == Dest::InPlace) {
            m_rememberedDest = Dest::InPlace;
        } else {
            m_rememberedDest = Dest::ToNew;
        }
    }

    emit planReady(p);
}


void mdn::gui::OpsController::runQuick(Op op, Dest dest) {
    QStringList names = collectTabNames();
    if (names.size() < 2) {
        runDialog(op);
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
    if (dest == Dest::ToNew) {
        QString sym = "+";
        if (op == Op::Subtract) {
            sym = "-";
        } else {
            if (op == Op::Multiply) {
                sym = "×";
            } else {
                sym = "÷";
            }
        }
        p.newName = names.value(a) + " " + sym + " " + names.value(b);
    }
    emit planReady(p);
}


mdn::gui::OpsController::Dest mdn::gui::OpsController::stripDestToController(
    OperationStrip::DestinationMode d
) const {
    if (d == OperationStrip::DestinationMode::InPlace) {
        return Dest::InPlace;
    } else {
        return Dest::ToNew;
    }
}


mdn::gui::OpsController::Op mdn::gui::OpsController::stripOpToController(
    OperationStrip::Operation o
) const {
    if (o == OperationStrip::Operation::Add) {
        return Op::Add;
    } else {
        if (o == OperationStrip::Operation::Subtract) {
            return Op::Subtract;
        } else {
            if (o == OperationStrip::Operation::Multiply) {
                return Op::Multiply;
            } else {
                return Op::Divide;
            }
        }
    }
}
