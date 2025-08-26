#include "OperationStrip.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

OperationStrip::OperationStrip(QWidget* parent)
    : QWidget(parent) {
    QHBoxLayout* lay = new QHBoxLayout(this);
    lay->setContentsMargins(6, 6, 6, 6);
    lay->setSpacing(8);

    m_add = new QPushButton(tr("Add"), this);
    m_sub = new QPushButton(tr("Subtract"), this);
    m_mul = new QPushButton(tr("Multiply"), this);
    m_div = new QPushButton(tr("Divide"), this);

    connect(m_add, SIGNAL(clicked()), this, SLOT(onClickAdd()));
    connect(m_sub, SIGNAL(clicked()), this, SLOT(onClickSub()));
    connect(m_mul, SIGNAL(clicked()), this, SLOT(onClickMul()));
    connect(m_div, SIGNAL(clicked()), this, SLOT(onClickDiv()));

    QLabel* bLabel = new QLabel(tr("B:"), this);
    m_bPicker = new QComboBox(this);
    m_bPicker->setMinimumContentsLength(10);

    QPushButton* changeB = new QPushButton(tr("Change…"), this);
    connect(changeB, SIGNAL(clicked()), this, SLOT(onChangeB()));

    QLabel* aLabel = new QLabel(tr("A:"), this);
    m_labelA = new QLabel(tr(""), this);

    m_destPicker = new QComboBox(this);
    m_destPicker->addItem(tr("In place"), static_cast<int>(DestinationMode::InPlace));
    m_destPicker->addItem(tr("To new"), static_cast<int>(DestinationMode::ToNew));
    connect(m_destPicker, SIGNAL(currentIndexChanged(int)), this, SLOT(onDestChanged(int)));

    lay->addWidget(m_add);
    lay->addWidget(m_sub);
    lay->addWidget(m_mul);
    lay->addWidget(m_div);
    lay->addSpacing(12);
    lay->addWidget(bLabel);
    lay->addWidget(m_bPicker);
    lay->addWidget(changeB);
    lay->addSpacing(12);
    lay->addWidget(aLabel);
    lay->addWidget(m_labelA);
    lay->addSpacing(12);
    lay->addWidget(m_destPicker);
    lay->addStretch(1);
}

void OperationStrip::setTabNames(const QStringList& names) {
    m_names = names;
    m_bPicker->clear();
    m_bPicker->addItems(m_names);
    if (m_indexB >= 0 && m_indexB < m_names.size()) {
        m_bPicker->setCurrentIndex(m_indexB);
    }
}

void OperationStrip::setActiveIndex(int indexA) {
    m_indexA = indexA;
    m_labelA->setText(m_names.value(m_indexA));
}

void OperationStrip::setRememberedB(int indexB) {
    m_indexB = indexB;
    if (m_indexB >= 0 && m_indexB < m_names.size()) {
        m_bPicker->setCurrentIndex(m_indexB);
    }
}

void OperationStrip::setDestinationMode(DestinationMode mode) {
    int idx = 0;
    if (mode == DestinationMode::InPlace) {
        idx = 0;
    } else {
        idx = 1;
    }
    m_destPicker->setCurrentIndex(idx);
}

void OperationStrip::onClickAdd() {
    emitOp(Operation::Add);
}

void OperationStrip::onClickSub() {
    emitOp(Operation::Subtract);
}

void OperationStrip::onClickMul() {
    emitOp(Operation::Multiply);
}

void OperationStrip::onClickDiv() {
    emitOp(Operation::Divide);
}

void OperationStrip::onChangeB() {
    emit requestChangeB();
}

void OperationStrip::onDestChanged(int) {
}

void OperationStrip::emitOp(OperationStrip::Operation op) {
    int idxB = m_bPicker->currentIndex();
    OperationStrip::DestinationMode dest = OperationStrip::DestinationMode::InPlace;
    int stored = m_destPicker->currentIndex();
    if (stored == 1) {
        dest = OperationStrip::DestinationMode::ToNew;
    }
    emit requestOperation(op, m_indexA, idxB, dest);
}
