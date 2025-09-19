#include "OperationStrip.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>

mdn::gui::OperationStrip::OperationStrip(QWidget* parent)
:
    QWidget(parent)
{
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(0,0,0,0);
    row->setSpacing(6);

    m_btnCancel = makeButton(tr("Cancel"));
    m_btnCancel->setEnabled(false);
    row->addWidget(m_btnCancel);
    row->addSpacing(8);

    addOpButton(row, Operation::Add);
    addOpButton(row, Operation::Subtract);
    addOpButton(row, Operation::Multiply);
    addOpButton(row, Operation::Divide);

    row->addSpacing(8);
    m_btnNewTab = makeButton(tr("NewTab"));
    row->addWidget(m_btnNewTab);

    connect(m_btnCancel, &QToolButton::clicked, this, [this]{ emit requestCancel(); });
    connect(m_btnNewTab, &QToolButton::clicked, this, [this]{ emit requestNewTab(); });
    setLayout(row);
}


// void mdn::gui::OperationStrip::onClickAdd() {
//     emitOp(Operation::Add);
// }


// void mdn::gui::OperationStrip::onClickSub() {
//     emitOp(Operation::Subtract);
// }


// void mdn::gui::OperationStrip::onClickMul() {
//     emitOp(Operation::Multiply);
// }


// void mdn::gui::OperationStrip::onClickDiv() {
//     emitOp(Operation::Divide);
// }


// void mdn::gui::OperationStrip::emitOp(Operation op) {
//     int idxB = m_bPicker->currentIndex();
//     DestinationSimple dest = DestinationSimple::InPlace;
//     int stored = m_destPicker->currentIndex();
//     if (stored == 1) {
//         dest = DestinationSimple::ToNew;
//     }
//     emit requestOperation(op, m_indexA, idxB, dest);
// }

QToolButton* mdn::gui::OperationStrip::makeButton(const QString& text) {
    auto* b = new QToolButton;
    b->setText(text);
    b->setAutoRaise(true);
    return b;
}


void mdn::gui::OperationStrip::addOpButton(QHBoxLayout* row, Operation op) {
    auto* b = makeButton(QString::fromStdString(OperationToOpStr(op)));
    b->setToolTip(QString::fromStdString(OperationToString(op)));
    connect(b, &QToolButton::clicked, this, [this, op]{ emit requestOperation(op); });
    row->addWidget(b);
}
