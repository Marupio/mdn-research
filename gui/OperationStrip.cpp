#include "OperationStrip.hpp"

#include <QToolButton>

#include "MdnQtInterface.hpp"

mdn::gui::OperationStrip::OperationStrip(QWidget* parent)
    : QWidget(parent)
{
    Log_Debug2_H("");
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(6);
    m_btnCancel = new QPushButton(tr("Cancel"), this);
    // Compact op buttons as toolbuttons
    auto mkOpBtn = [this](const QString& text, const QString& tip) {
        auto* b = new QToolButton(this);
        b->setText(text);
        b->setToolTip(tip);
        b->setToolButtonStyle(Qt::ToolButtonTextOnly);
        b->setAutoRaise(true);
        b->setCheckable(true);
        b->setFocusPolicy(Qt::NoFocus);
        return b;
    };
    m_btnAdd = mkOpBtn(
        MdnQtInterface::toQString(OperationToOpStr(Operation::Add)),
        tr("Add")
    );
    m_btnSub = mkOpBtn(
        MdnQtInterface::toQString(OperationToOpStr(Operation::Subtract)),
        tr("Subtract")
    );
    m_btnMul = mkOpBtn(
        MdnQtInterface::toQString(OperationToOpStr(Operation::Multiply)),
        tr("Multiply")
    );
    m_btnDiv = mkOpBtn(
        MdnQtInterface::toQString(OperationToOpStr(Operation::Divide)),
        tr("Divide")
    );

    // Convenience
    m_allOpButtons = { m_btnAdd, m_btnSub, m_btnMul, m_btnDiv };

    // Wire user intent -> OpsController (OpsController will call activate/reset)
    connect(m_btnAdd, &QToolButton::clicked, this, &OperationStrip::onAdd);
    connect(m_btnSub, &QToolButton::clicked, this, &OperationStrip::onSub);
    connect(m_btnMul, &QToolButton::clicked, this, &OperationStrip::onMul);
    connect(m_btnDiv, &QToolButton::clicked, this, &OperationStrip::onDiv);
    connect(m_btnCancel, &QPushButton::clicked, this, &OperationStrip::onCancel);

    // Layout: [Cancel] [+] [–] [×] [÷] [T↔] [c/o] [c/+] [c/-]
    // lay->addSpacing(8);

    m_btnTranspose = new QToolButton(this);
    m_btnTranspose->setAutoRaise(true);
    m_btnTranspose->setToolTip(tr("Transpose"));
    QIcon ticon = QIcon::fromTheme("transpose-move");
    if (!ticon.isNull()) {
        m_btnTranspose->setIcon(ticon);
    } else {
        m_btnTranspose->setText(QStringLiteral("T↔"));
    }

    connect(m_btnTranspose, &QToolButton::clicked, this, &OperationStrip::transposeClicked);

    // New carry-over buttons (text-only, compact)
    auto mkSmallBtn = [this](const QString& text, const QString& tip) {
        auto* b = new QToolButton(this);
        b->setText(text);
        b->setToolTip(tip);
        b->setToolButtonStyle(Qt::ToolButtonTextOnly);
        b->setAutoRaise(true);
        b->setFocusPolicy(Qt::NoFocus);
        return b;
    };
    m_btnCarryOver = mkSmallBtn(QStringLiteral("c/o"), tr("Carry-over this digit"));
    m_btnCarryPos  = mkSmallBtn(QStringLiteral("c/+"), tr("Carry-over all → positive"));
    m_btnCarryNeg  = mkSmallBtn(QStringLiteral("c/-"), tr("Carry-over all → negative"));
    connect(m_btnCarryOver, &QToolButton::clicked, this, &OperationStrip::carryOverClicked);
    connect(m_btnCarryPos,  &QToolButton::clicked, this, &OperationStrip::carryPosClicked);
    connect(m_btnCarryNeg,  &QToolButton::clicked, this, &OperationStrip::carryNegClicked);

    // Compute a uniform width for all compact buttons based on the widest label
    QStringList labels;
    labels << m_btnAdd->text() << m_btnSub->text() << m_btnMul->text() << m_btnDiv->text();
    labels << (m_btnTranspose->text().isEmpty() ? QStringLiteral("T↔") : m_btnTranspose->text());
    labels << m_btnCarryOver->text() << m_btnCarryPos->text() << m_btnCarryNeg->text();
    QFontMetrics fm(font());
    int maxTextW = 0;
    for (const auto& s : labels) maxTextW = std::max(maxTextW, fm.horizontalAdvance(s));
    // Add a little padding so text isn’t cramped (style may add a bit more)
    const int uniformW = maxTextW + 20;
    auto applyFixedW = [uniformW](QToolButton* b){
        b->setFixedWidth(uniformW);
        b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    };
    for (auto* b : m_allOpButtons) applyFixedW(b);
    applyFixedW(m_btnTranspose);
    applyFixedW(m_btnCarryOver);
    applyFixedW(m_btnCarryPos);
    applyFixedW(m_btnCarryNeg);

    lay->addWidget(m_btnCancel);
    lay->addSpacing(8);
    lay->addWidget(m_btnAdd);
    lay->addWidget(m_btnSub);
    lay->addWidget(m_btnMul);
    lay->addWidget(m_btnDiv);
    lay->addWidget(m_btnTranspose);
    lay->addWidget(m_btnCarryOver);
    lay->addWidget(m_btnCarryPos);
    lay->addWidget(m_btnCarryNeg);
    lay->addStretch(1);

    auto tagOpBtn = [](QToolButton* b){
        b->setProperty("opstyle", true);
        // show border even when not hovered
        b->setAutoRaise(false);
        // optional: allow keyboard focus
        b->setFocusPolicy(Qt::TabFocus);
    };

    for (
        auto* b :
        {
            m_btnAdd,
            m_btnSub,
            m_btnMul,
            m_btnDiv,
            m_btnTranspose,
            m_btnCarryOver,
            m_btnCarryPos,
            m_btnCarryNeg
        }
    ) {
        tagOpBtn(b);
    }

    const QString opCss = R"(
    QToolButton[opstyle="true"] {
    padding: 2px 6px;
    border: 1px solid palette(mid);
    border-radius: 6px;                     /* pill look? use 999px */
    background-color: palette(alternate-base);
    color: palette(button-text);
    }

    QToolButton[opstyle="true"]:hover {
    background-color: palette(button);
    }

    QToolButton[opstyle="true"]:pressed {
    background-color: palette(midlight);
    }

    QToolButton[opstyle="true"]:checked {
    background-color: palette(highlight);
    color: palette(highlighted-text);
    border-color: palette(highlight);
    }

    QToolButton[opstyle="true"]:disabled {
    color: palette(mid);
    border-color: palette(midlight);
    background-color: palette(window);
    }

    /* Optional: clearer keyboard focus */
    QToolButton[opstyle="true"]:focus {
    outline: none;
    border: 1px solid palette(highlight);
    }
    )";
    this->setStyleSheet(opCss);

    // Idle state by default
    reset();
    Log_Debug2_T("");
}


void mdn::gui::OperationStrip::battlestations(Operation op) {
    Log_Debug2_H("op=" << op);
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
    Log_Debug2_T("");
}


void mdn::gui::OperationStrip::reset() {
    // Cancel disabled; all ops enabled; none checked
    Log_Debug2_H("");
    m_btnCancel->setEnabled(false);
    setOpsEnabled(true);

    // Clear checked state without breaking exclusivity
    for (QToolButton* b : { m_btnAdd, m_btnSub, m_btnMul, m_btnDiv }) {
        b->setChecked(false);
    }
    // Clear ActiveDivision visuals if any
    leaveActiveDivisionVisual();
    Log_Debug2_T("");
}


void mdn::gui::OperationStrip::enterActiveDivisionVisual() {
    if (m_activeDivision) {
        return;
    }
    m_activeDivision = true;
    // Make sure Divide is enabled and popped (unchecked)
    m_btnDiv->setEnabled(true);
    m_btnDiv->setChecked(false);

    // soft amber
    m_btnDiv->setStyleSheet("QToolButton { background: rgba(255, 215, 0, 0.25); }");
}


void mdn::gui::OperationStrip::leaveActiveDivisionVisual() {
    if (!m_activeDivision) {
        return;
    }
    m_activeDivision = false;

    // back to theme defaults
    m_btnDiv->setStyleSheet(QString());
}


void mdn::gui::OperationStrip::setOpsEnabled(bool enabled) {
    Log_Debug2_H("enabled=" << enabled);
    for (QToolButton* b : { m_btnAdd, m_btnSub, m_btnMul, m_btnDiv }) {
        b->setEnabled(enabled);
    }
    Log_Debug2_T("");
}


QToolButton* mdn::gui::OperationStrip::buttonFor(Operation op) const {
    Log_Debug2_H("");
    switch (op) {
        case Operation::Add: {
            Log_Debug_T("Add");
            return m_btnAdd;
        }
        case Operation::Subtract: {
            Log_Debug_T("Subtract");
            return m_btnSub;
        }
        case Operation::Multiply: {
            Log_Debug_T("Multiply");
            return m_btnMul;
        }
        case Operation::Divide: {
            Log_Debug_T("Divide");
            return m_btnDiv;
        }
        default: {
            Log_Debug_T("default");
            return nullptr;
        }
    }
    Log_Debug2_T("");
}


void mdn::gui::OperationStrip::setOthersDisabledExcept(Operation op) {
    Log_Debug2_H("op=" << op);
    for (auto* b : m_allOpButtons) {
        if (b) { b->setEnabled(false); b->setChecked(false); }
    }
    if (auto* keep = buttonFor(op)) {
        keep->setEnabled(true); keep->setChecked(true);
    }
    Log_Debug2_T("");
}


void mdn::gui::OperationStrip::onDiv() {
    if (m_activeDivision) {
        Log_Debug3("emit divisionIterateClicked()");
        emit divisionIterateClicked();
    } else {
        Log_Debug3("emit operationClicked(Divide)");
        emit operationClicked(Operation::Divide);
    }
}


void mdn::gui::OperationStrip::onCancel() {
    if (m_activeDivision) {
        Log_Debug3("emit divisionStopRequested()");
        emit divisionStopRequested();
    } else {
        Log_Debug3("emit cancelClicked()");
        emit cancelClicked();
    }
}
