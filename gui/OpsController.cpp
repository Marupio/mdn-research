#include "MdnQtInterface.hpp"
#include "OpsController.hpp"
#include "Project.hpp"
#include "HoverPeekTabWidget.hpp"

mdn::gui::OpsController::OpsController(Project* proj, HoverPeekTabWidget* tabs, QObject* parent)
    : QObject(parent), m_project(proj), m_tabs(tabs) {
    connect(m_tabs, &HoverPeekTabWidget::committedIndex,
            this,   &OpsController::onTabCommitted);
}


void mdn::gui::OpsController::startAdd() {
    begin(Operation::Add);
}


void mdn::gui::OpsController::startSub() {
    begin(Operation::Subtract);
}


void mdn::gui::OpsController::startMul() {
    begin(Operation::Multiply);
}


void mdn::gui::OpsController::startDiv() {
    begin(Operation::Divide);
}


void mdn::gui::OpsController::begin(Operation op) {
    if (!m_project || !m_tabs) {
        return;
    }
    m_op = op;
    // Operand A is current
    m_a = m_tabs->currentIndex();
    m_b = -1;
    m_phase = Phase::PickB;
    emit requestCancelEnabled(true);
    std::string status = m_project->nameOfMdn(m_a) + " " + OperationToOpStr(m_op) + " *Choose*";
    QString qStatus = MdnQtInterface::toQString(status);
    emit requestStatus(qStatus);
}


void mdn::gui::OpsController::cancel() {
    m_phase = Phase::Idle; m_a = m_b = -1;
    emit requestCancelEnabled(false);
    emit requestStatus(QString());
}


void mdn::gui::OpsController::onTabCommitted(int idx) {
    if (!m_project || m_phase == Phase::Idle) return;

    if (m_phase == Phase::PickB) {
        if (idx == m_a) return; // don’t allow same tab for B; ignore click
        m_b = idx;
        m_phase = Phase::PickDest;
        std::string status(
            m_project->nameOfMdn(m_a) + " "
            + OperationToOpStr(m_op) + " "
            + m_project->nameOfMdn(m_b) + "  = *Choose*"
        );
        QString qStatus = MdnQtInterface::toQString(status);
        emit requestStatus(qStatus);
        return;
    }

    if (m_phase == Phase::PickDest) {
        // Destination = overwrite existing tab
        finishTo(idx, false);
    }
}


void mdn::gui::OpsController::onNewTab() {
    if (m_phase != Phase::PickDest) return;
    std::string defaultName(
        m_project->nameOfMdn(m_a) + OperationToOpStr(m_op) + m_project->nameOfMdn(m_b)
    );
    QString qDefaultName = MdnQtInterface::toQString(defaultName);
    emit requestNewTab(qDefaultName);
    int idx = m_tabs->currentIndex();
    finishTo(idx, true);
}


void OpsController::finishTo(int destIndex, bool /*isNew*/) {
    // Perform the operation; adapt to your existing project API.
    // Example pseudocode: m_project->applyBinary(m_op, m_a, m_b, destIndex);
    // If your API uses DestinationMode:
    // m_project->applyBinary(m_op, m_a, m_b, EnumDestinationMode::Overwrite, destIndex);

    // --- Integrate your actual call here ---
    // m_project->applyBinary(m_op, m_a, m_b, EnumDestinationMode::Overwrite, destIndex);

    // UX clean-up
    QString report = QString("%1 %2 %3  →  %4")
        .arg(QString::fromStdString(m_project->nameOfMdn(m_a)))
        .arg(ENUM_OPERATION_ToSymbol(m_op))
        .arg(QString::fromStdString(m_project->nameOfMdn(m_b)))
        .arg(QString::fromStdString(m_project->nameOfMdn(destIndex)));
    emit requestStatus(report);

    m_phase = Phase::Idle; m_a = m_b = -1;
    emit requestCancelEnabled(false);
}
