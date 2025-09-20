#include "OperationStrip.hpp"

#include "MdnQtInterface.hpp"

mdn::gui::OperationStrip::OperationStrip(QWidget* parent)
    : QWidget(parent)
{
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(6);

    m_btnCancel = new QPushButton(tr("Cancel"), this);
    m_btnAdd    = new QPushButton(
        MdnQtInterface::toQString(OperationToOpStr(Operation::Add)),
        this
    );
    m_btnSub    = new QPushButton(
        MdnQtInterface::toQString(OperationToOpStr(Operation::Subtract)),
        this
    );
    m_btnMul    = new QPushButton(
        MdnQtInterface::toQString(OperationToOpStr(Operation::Multiply)),
        this
    );
    m_btnDiv    = new QPushButton(
        MdnQtInterface::toQString(OperationToOpStr(Operation::Divide)),
        this
    );

    // Convenience
    m_allOpButtons = { m_btnAdd, m_btnSub, m_btnMul, m_btnDiv };

    // Make op buttons checkable so the active one looks “clicked in”
    for (QPushButton* b : { m_btnAdd, m_btnSub, m_btnMul, m_btnDiv }) {
        b->setCheckable(true);
        b->setFocusPolicy(Qt::NoFocus);
        b->setMinimumWidth(28);
    }

    // Group is exclusive so the checked one cannot be unchecked by clicking again
    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);
    m_group->addButton(m_btnAdd, static_cast<int>(Operation::Add));
    m_group->addButton(m_btnSub, static_cast<int>(Operation::Subtract));
    m_group->addButton(m_btnMul, static_cast<int>(Operation::Multiply));
    m_group->addButton(m_btnDiv, static_cast<int>(Operation::Divide));

    // Wire user intent -> OpsController (OpsController will call activate/reset)
    connect(m_btnAdd, &QPushButton::clicked, this, &OperationStrip::onAdd);
    connect(m_btnSub, &QPushButton::clicked, this, &OperationStrip::onSub);
    connect(m_btnMul, &QPushButton::clicked, this, &OperationStrip::onMul);
    connect(m_btnDiv, &QPushButton::clicked, this, &OperationStrip::onDiv);
    connect(m_btnCancel, &QPushButton::clicked, this, &OperationStrip::onCancel);

    // Layout: [Cancel] [+] [–] [×] [÷]
    lay->addWidget(m_btnCancel);
    lay->addSpacing(8);
    lay->addWidget(m_btnAdd);
    lay->addWidget(m_btnSub);
    lay->addWidget(m_btnMul);
    lay->addWidget(m_btnDiv);
    lay->addStretch(1);

    // Idle state by default
    reset();
}


void mdn::gui::OperationStrip::battlestations(Operation op)
{
    // Enable Cancel; lock the chosen op “in”
    m_btnCancel->setEnabled(true);

    // Disable all other ops; chosen one enabled + checked
    setOthersDisabledExcept(op);

    // Mark as checked (stays checked; exclusive group prevents un-check)
    if (auto* chosen = buttonFor(op)) {
        chosen->setChecked(true);
        // Optional: visual emphasis even when disabled peers are grey —
        // you can tweak stylesheet if you want stronger “pressed” look.
    }
}


QPushButton* mdn::gui::OperationStrip::buttonFor(Operation op) const
{
    switch (op) {
        case Operation::Add: return m_btnAdd;
        case Operation::Subtract: return m_btnSub;
        case Operation::Multiply: return m_btnMul;
        case Operation::Divide: return m_btnDiv;
        default:             return nullptr;
    }
}


void mdn::gui::OperationStrip::setOpsEnabled(bool enabled)
{
    for (QPushButton* b : { m_btnAdd, m_btnSub, m_btnMul, m_btnDiv }) {
        b->setEnabled(enabled);
    }
}


void mdn::gui::OperationStrip::setOthersDisabledExcept(Operation op)
{
    for (auto* b : m_allOpButtons) {
        if (b) { b->setEnabled(false); b->setChecked(false); }
    }
    if (auto* keep = buttonFor(op)) {
        keep->setEnabled(true); keep->setChecked(true);
    }
}


void mdn::gui::OperationStrip::reset()
{
    // Cancel disabled; all ops enabled; none checked
    m_btnCancel->setEnabled(false);
    setOpsEnabled(true);

    // Clear checked state without breaking exclusivity
    for (QPushButton* b : { m_btnAdd, m_btnSub, m_btnMul, m_btnDiv }) {
        b->setChecked(false);
    }
}
