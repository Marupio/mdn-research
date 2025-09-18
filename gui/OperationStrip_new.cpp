#include "OperationStrip.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

mdn::gui::OperationStrip::OperationStrip(QWidget* parent)
:
    QWidget(parent)
{
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(0,0,0,0);
    row->setSpacing(6);

    btnCancel  = makeButton(tr("Cancel"));
    btnCancel->setEnabled(false);

    makeOpButton(row, Operation::Add);
    makeOpButton(row, Operation::Subtract);
    makeOpButton(row, Operation::Multiply);
    makeOpButton(row, Operation::Divide);

    btnNewTab = makeButton(tr("NewTab"));

    row->insertWidget(0, btnCancel);
    row->addSpacing(8);
    row->addWidget(btnNewTab);
    setLayout(row);

    connect(btnCancel, &QToolButton::clicked, this, [this]{ emit requestCancel(); });
    connect(btnNewTab, &QToolButton::clicked, this, [this]{ emit requestNewTab(); });
}


QToolButton* mdn::gui::OperationStrip::makeButton(const QString& text) {
    auto* b = new QToolButton;
    b->setText(text);
    b->setAutoRaise(true);
    return b;
}
void mdn::gui::OperationStrip::makeOpButton(QHBoxLayout* row, Operation op) {
    auto* b = makeButton(QString::fromStdString(OperationToOpStr(op)));
    b->setToolTip(QString::fromStdString(OperationToString(op)));
    connect(b, &QToolButton::clicked, this, [this, op]{ emit requestOperation(op); });
    row->addWidget(b);
}


void mdn::gui::OperationStrip::onClickAdd() {
    emitOp(Operation::Add);
}


void mdn::gui::OperationStrip::onClickSub() {
    emitOp(Operation::Subtract);
}


void mdn::gui::OperationStrip::onClickMul() {
    emitOp(Operation::Multiply);
}


void mdn::gui::OperationStrip::onClickDiv() {
    emitOp(Operation::Divide);
}


void mdn::gui::OperationStrip::onChangeB() {
    emit requestChangeB();
}


void mdn::gui::OperationStrip::onDestChanged(int) {
}


void mdn::gui::OperationStrip::emitOp(Operation op) {
    int idxB = m_bPicker->currentIndex();
    DestinationSimple dest = DestinationSimple::InPlace;
    int stored = m_destPicker->currentIndex();
    if (stored == 1) {
        dest = DestinationSimple::ToNew;
    }
    emit requestOperation(op, m_indexA, idxB, dest);
}
