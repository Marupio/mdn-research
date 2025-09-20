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
#include "Project.hpp"
#include "HoverPeekTabWidget.hpp"
#include "CommandWidget.hpp"

mdn::gui::OpsController::OpsController(
    QMainWindow* mw,
    Project* project,
    HoverPeekTabWidget* tabs,
    CommandWidget* command,
    QObject* parent
) :
    QObject(parent),
    m_mainWindow(mw),
    m_project(project),
    m_tabs(tabs),
    m_command(command)
{
    Log_Debug3_H("");
    buildMenus();
    rebuildBottomContainer();
    m_strip = new OperationStrip(m_bottomContainer);
    if (m_strip) {
        connect(m_strip, &OperationStrip::operationClicked, this, [this](Operation op){
            battlestations(op);
        });
        connect(m_strip, &OperationStrip::cancelClicked, this, &OpsController::onCancel);
    }
    // choose B / destination via tab clicks
    if (m_tabs) {
        connect(
            m_tabs,
            &HoverPeekTabWidget::committedIndex,
            this,
            &OpsController::onTabCommitted
        );
        connect(
            m_tabs,
            &HoverPeekTabWidget::plusClicked,
            this,
            &OpsController::onPlusClicked
        );
    }
    Log_Debug3_T("");
}


QWidget* mdn::gui::OpsController::bottomContainer() const {
    return m_bottomContainer;
}


bool mdn::gui::OpsController::inBattle() const {
    return m_phase != OperationPhase::Idle;
}


void mdn::gui::OpsController::resetModel(Project* project) {
    m_project = project;
}


void mdn::gui::OpsController::clearModel() {
    m_project = nullptr;
}


void mdn::gui::OpsController::battlestations(Operation op) {
    m_strip->battlestations(op);
    // TODO WIRE IN OUR FANCY NEW INTERFACE
}


void mdn::gui::OpsController::onCancel() {
    cancel();
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


void mdn::gui::OpsController::onTabCommitted(int idx) {
    switch (m_phase) {
        case OperationPhase::Idle: {
            // Relay signal onwards
            emit tabClicked(idx);
            return;
        }
        case OperationPhase::PickB: {
            AssertQ(m_project && m_tabs, "Missing project or tabs, internal error");
            m_b = idx;
            m_phase = OperationPhase::PickDest;
            emit requestStatus(QString("%1 %2 %3  =  *Choose*")
                .arg(nameFor(m_a))
                .arg(QString::fromStdString(OperationToOpStr(m_op)))
                .arg(nameFor(m_b)), 0
            );
            return;
        }
        case OperationPhase::PickDest: {
            endBattle(idx, false);
            return;
        }
    }
}


void mdn::gui::OpsController::onPlusClicked() {
    switch (m_phase) {
        case OperationPhase::Idle: {
            // Relay signal onwards
            emit plusClicked();
            return;
        }
        case OperationPhase::PickB: {
            // Clicking plusTab during 'pickB' phase - do nothing
            return;
        }
        case OperationPhase::PickDest: {
            endBattle(-1, true);
            return;
        }
    }
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
    QWidget* parent = m_command->parentWidget();
    m_bottomContainer = new QWidget(parent);

    QVBoxLayout* lay = new QVBoxLayout(m_bottomContainer);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    m_strip = new OperationStrip(m_bottomContainer);
    lay->addWidget(m_strip);

    m_command->setParent(m_bottomContainer);
    lay->addWidget(m_command, 1);
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


QString mdn::gui::OpsController::nameFor(int tabIndex) const {
    if (!m_project) {
        return {};
    }
    return QString::fromStdString(m_project->nameOfMdn(tabIndex));
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
    QStringList names = collectTabNames();
    if (names.size() < 2) {
        Log_Debug3_T("names.size() = " << names.size());
        return;
    }
    BinaryOperationDialog dlg(m_mainWindow);

    dlg.setTabNames(names);
    dlg.setActiveIndex(activeIndex());

    dlg.setInitialOperation(preset);

    int b = m_rememberedB;
    if (b < 0 || b >= names.size()) {
        b = 0;
    }
    dlg.setRememberedB(b);

    DestinationMode d = DestinationMode::OverwriteA;
    if (m_rememberedDest == DestinationSimple::InPlace) {
        d = DestinationMode::OverwriteA;
    } else {
        d = DestinationMode::CreateNew;
    }
    dlg.setRememberedDestination(d);

    int res = dlg.exec();
    if (res != QDialog::Accepted) {
        Log_Debug3_T("Not accepted");
        return;
    }

    OperationPlan p = dlg.plan();
    Log_Debug4("p=" << p);

    // if (pp.rememberChoices) {
    //     m_rememberedB = pp.indexB;
    //     if (p.dest == DestinationSimple::InPlace) {
    //         m_rememberedDest = DestinationSimple::InPlace;
    //     } else {
    //         m_rememberedDest = DestinationSimple::ToNew;
    //     }
    // }

    Log_Debug3_T("Emitting " << p);
    emit planReady(p);
}


void mdn::gui::OpsController::endBattle(int destIndex, bool isNew) {
    if (m_a < 0 || m_b < 0) {
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

    OperationPlan p;
    p.op = m_op;
    p.indexA = m_a;
    p.indexB = m_b;
    p.indexDest = destIndex;
    p.newName = defaultName;

    emit planReady(p);

    m_phase = OperationPhase::Idle; m_a = m_b = -1;
    if (m_strip) {
        m_strip->reset();
    }
}


void mdn::gui::OpsController::cancel() {
    m_phase = OperationPhase::Idle; m_a = m_b = -1;
    if (m_strip) {
        m_strip->reset();
    }
    emit requestStatus(QString(), 0);
}
