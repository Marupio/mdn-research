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
#include "StatusDisplayWidget.hpp"

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


void mdn::gui::OpsController::setCommandVisible(bool on) {
    Log_Debug2_H("arg=" << on);
    if (!m_command) {
        Log_Debug2_T("no command");
        return;
    }
    m_command->setVisible(on);
    // collapse to zero height when off
    m_command->setMaximumHeight(on ? QWIDGETSIZE_MAX : 0);
    m_bottomContainer->updateGeometry();

    Log_Debug3("emit requestFitBottomToContents");
    emit requestFitBottomToContents();
    Log_Debug2_T("");
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


void mdn::gui::OpsController::enterActiveDivision() {
    Log_Debug2_H("");
    m_phase = OperationPhase::ActiveDivision;
    if (m_strip) {
        // In ActiveDivision, Divide is popped but highlighted and enabled
        m_strip->enterActiveDivisionVisual();
        // Keep other ops enabled; user can still switch ops via normal click flow if desired
        m_strip->setOpsEnabled(true);
    }
    // Optional: status hint
    emit requestStatus(QStringLiteral("Divide active — click ÷ to iterate; Esc/Cancel to stop"), 0, false);
    Log_Debug2_T("");
}


void mdn::gui::OpsController::leaveActiveDivision(bool showCancelMessage) {
    Log_Debug2_H("");
    m_phase = OperationPhase::Idle;
    if (m_strip) {
        m_strip->leaveActiveDivisionVisual();
        m_strip->reset(); // back to default: no op checked, all enabled, Cancel disabled
    }
    if (showCancelMessage) {
        emit requestStatus(QString(), 0, false);
        emit requestStatus(QStringLiteral("* Stopped division *"), 2000, false);
    }
    Log_Debug2_T("");
}


void mdn::gui::OpsController::battlestations(Operation op) {
    Log_Debug2_H("op=" << op);
    m_op = op;
    m_a = m_tabs->currentSelectedIndex();

    QString rqs(
        QString("%1 %2 *Operand B* >> Choose tab <<  Hover to peek")
        .arg(nameFor(m_a))
        .arg(QString::fromStdString(OperationToOpStr(m_op)))
    );
    Log_Debug3("emit requestStatus(rqs=[" << rqs.toStdString() << "], 0)");
    emit requestStatus(rqs, 0, false);

    m_strip->battlestations(op);
    m_phase = OperationPhase::PickB;
    Log_Debug2_T("");
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
    Log_Debug2_H("idx=" << idx << ",phase=" << OperationPhaseToString(m_phase) << ")");
    switch (m_phase) {
        case OperationPhase::Idle: {
            // Relay signal onwards
            Log_Debug3("emit tabClicked(idx=" << idx << ")");
            emit tabClicked(idx);
            Log_Debug2_T("");
            return;
        }
        case OperationPhase::PickB: {
            AssertQ(m_project && m_tabs, "Missing project or tabs, internal error");
            m_b = idx;
            if (m_op != Operation::Divide) {
                m_phase = OperationPhase::PickDest;
                QString rqs(
                    QString("%1 %2 %3  →  *Destination* >> Choose tab <<  Hover to peek")
                    .arg(nameFor(m_a))
                    .arg(QString::fromStdString(OperationToOpStr(m_op)))
                    .arg(nameFor(m_b))
                );
                Log_Debug3("emit requestStatus(rqs=[" << rqs.toStdString() << "], 0)");
                emit requestStatus(rqs, 0, false);
                Log_Debug2_T("");
                return;
            } else {
                m_phase = OperationPhase::PickRem;
                QString rqs(
                    QString("%1 %2 %3 rem( *Remainder* ) >> Choose tab <<  Hover to peek")
                    .arg(nameFor(m_a))
                    .arg(QString::fromStdString(OperationToOpStr(m_op)))
                    .arg(nameFor(m_b))
                );
                Log_Debug3("emit requestStatus(rqs=[" << rqs.toStdString() << "], 0)");
                emit requestStatus(rqs, 0, false);
                Log_Debug2_T("");
                return;
            }
        }
        case OperationPhase::PickRem: {
            AssertQ(m_op == Operation::Divide, "Internal error: PickRem state requires op Divide");
            m_rem = idx;
            m_phase = OperationPhase::PickDest;
            QString rqs(
                QString("%1 %2 %3 rem(%4) →  *Destination* >> Choose tab <<  Hover to peek")
                .arg(nameFor(m_a))
                .arg(QString::fromStdString(OperationToOpStr(m_op)))
                .arg(nameFor(m_b))
                .arg(nameFor(m_rem))
            );
            Log_Debug3("emit requestStatus(rqs=[" << rqs.toStdString() << "], 0)");
            emit requestStatus(rqs, 0, false);
            Log_Debug2_T("");
            return;


        }
        case OperationPhase::PickDest: {
            m_dest = idx;
            endBattle();
            Log_Debug2_T("");
            return;
        }
    }
    Log_Debug2_T("");
}


void mdn::gui::OpsController::onPlusClicked() {
    Log_Debug2_H("");
    switch (m_phase) {
        case OperationPhase::Idle: {
            // Relay signal onwards
            Log_Debug3("emit plusClicked");
            emit plusClicked();
            Log_Debug2_T("");
            return;
        }
        case OperationPhase::PickB: {
            // Clicking plusTab during 'pickB' phase - do nothing
            Log_Debug2_T("");
            return;
        }
        case OperationPhase::PickRem: {
            m_phase = OperationPhase::PickDest;
            m_rem = -1;
            QString rqs(
                QString("%1 %2 %3 rem(%4)  →  *Destination* >> Choose tab <<  Hover to peek")
                .arg(nameFor(m_a))
                .arg(QString::fromStdString(OperationToOpStr(m_op)))
                .arg(nameFor(m_b))
                .arg(nameFor(m_rem))
            );
            Log_Debug3("emit requestStatus(rqs=[" << rqs.toStdString() << "], 0)");
            emit requestStatus(rqs, 0, false);
            Log_Debug2_T("");
            return;
        }
        case OperationPhase::PickDest: {
            m_dest = -1;
            endBattle();
            Log_Debug2_T("");
            return;
        }
    }
    Log_Debug2_T("");
}


void mdn::gui::OpsController::buildMenus() {
    Log_Debug2_H("");
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
    Log_Debug2_T("");
}


void mdn::gui::OpsController::rebuildBottomContainer() {
    Log_Debug2_H("");
    // QWidget* parent = m_command->parentWidget();
    // m_bottomContainer = new QWidget(parent);
    m_bottomContainer = new QWidget(m_mainWindow);

    QVBoxLayout* lay = new QVBoxLayout(m_bottomContainer);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    m_status = new StatusDisplayWidget(m_bottomContainer);
    m_status->setFontSize(11);
    m_status->showPermanentMessage("Ready.");
    lay->addWidget(m_status, 0);

    m_strip = new OperationStrip(m_bottomContainer);
    if (m_strip) {
        connect(m_strip, &OperationStrip::operationClicked, this, [this](Operation op){
            battlestations(op);
        });
        connect(m_strip, &OperationStrip::cancelClicked, this, &OpsController::onCancel);
        connect(
            m_strip, &OperationStrip::propertiesClicked,
            m_mainWindow,
            [this]() {
                QMetaObject::invokeMethod(
                    m_mainWindow,
                    "onProjectProperties",
                    Qt::QueuedConnection
                );
            }
        );
        connect(m_strip, &OperationStrip::divisionIterateClicked, this, [this]{
            if (m_phase == OperationPhase::ActiveDivision) {
                Log_Debug2_H("emit divisionIterateRequested()");
                emit divisionIterateRequested();
                Log_Debug2_T("");
            }
        });
        connect(m_strip, &OperationStrip::divisionStopRequested, this, [this]{
            if (m_phase == OperationPhase::ActiveDivision) {
                Log_Debug2_H("emit divisionStopRequested()");
                emit divisionStopRequested();
                leaveActiveDivision(false);
                Log_Debug2_T("");
            } else {
                onCancel();
            }
        });
    }
    lay->addWidget(m_strip, 0);

    if (m_command) {
        m_command->setParent(m_bottomContainer);
        lay->addWidget(m_command, 1);
    }
    Log_Debug2_T("");
}


QStringList mdn::gui::OpsController::collectTabNames() const {
    Log_Debug2_H("");
    QStringList out;
    if (!m_tabs) {
        Log_Debug2_T("");
        return out;
    }
    int skipPlusTab = m_tabs->count()-1;
    for (int i = 0; i < skipPlusTab; ++i) {
        out << m_tabs->tabText(i);
    }
    Log_Debug2_T("");
    return out;
}


QString mdn::gui::OpsController::nameFor(int tabIndex) const {
    Log_Debug2_H("tabIndex=" << tabIndex);
    if (!m_project) {
        Log_Debug2_T("");
        return {};
    }
    std::string retVal;
    if (tabIndex < 0) {
        retVal = "New";
    } else {
        retVal = m_project->nameOfMdn(tabIndex);
    }
    Log_Debug2_T("nameFor(" << tabIndex << ")='" << retVal << "'");
    return QString::fromStdString(retVal);
}


int mdn::gui::OpsController::activeIndex() const {
    Log_Debug2_H("");
    if (!m_tabs) {
        Log_Debug2_T("");
        return 0;
    }
    int idx = m_tabs->currentIndex();
    if (idx < 0) {
        idx = 0;
    }
    Log_Debug2_T("");
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

    Log_Debug3_T("emit planReady(plan=" << p << ")");
    emit planReady(p);
}


void mdn::gui::OpsController::endBattle() {
    Log_Debug2_H("");
    if (m_a < 0 || m_b < 0 || (m_rem < -1 && m_op == Operation::Divide)) {
        cancel();
        Log_Debug2_T("");
        return;
    }

    QString defaultRemName = "";
    if (m_op == Operation::Divide) {
        QString rqs(
            QString("%1 %2 %3 rem(%4)  →  %5")
            .arg(nameFor(m_a))
            .arg(QString::fromStdString(OperationToOpStr(m_op)))
            .arg(nameFor(m_b))
            .arg(nameFor(m_rem))
            .arg(nameFor(m_dest))
        );
        Log_Debug3("emit requestStatus(rqs=[" << rqs.toStdString() << "], 5000)");
        emit requestClearStatus();
        emit requestStatus(rqs, 5000, false);
        if (m_dest < 0) {
            defaultRemName = QString(
                QString("rem(%1%2%3)")
                    .arg(nameFor(m_a))
                    .arg(QString::fromStdString(OperationToOpStr(m_op)))
                    .arg(nameFor(m_b)
                )
            );
        }
    } else {
        QString rqs(
            QString("%1 %2 %3  →  %4")
            .arg(nameFor(m_a))
            .arg(QString::fromStdString(OperationToOpStr(m_op)))
            .arg(nameFor(m_b))
            .arg(nameFor(m_dest))
        );
        Log_Debug3("emit requestStatus(rqs=[" << rqs.toStdString() << "], 0)");
        emit requestClearStatus();
        emit requestStatus(rqs, 5000, false);
    }
    QString defaultDestName = "";
    if (m_dest < 0) {
        defaultDestName = QString(
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
    p.indexDest = m_dest;
    p.newName = defaultDestName;
    p.indexRem = m_rem;
    p.newRemName = defaultRemName;

    cancel(false);

    Log_Debug3("emit planReady(plan=" << p << ")");
    emit planReady(p);

    Log_Debug2_T("");
}


void mdn::gui::OpsController::cancel(bool showCancelMessage) {
    Log_Debug2_H("");
    m_phase = OperationPhase::Idle;
    m_a = -2;
    m_b = -2;
    m_dest = -2;
    m_rem = -2;
    if (m_strip) {
        m_strip->reset();
    }
    Log_Debug3("emit requestStatus([], 0)");
    if (showCancelMessage) {
        emit requestStatus(QString(), 0, false);
        emit requestStatus(QString("* Cancelled *"), 2000, false);
    }
    Log_Debug2_T("");
}
