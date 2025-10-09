#pragma once

#include <array>

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QSpinBox>
#include <QToolButton>
#include <QPushButton>
#include <QWidget>

#include <mdn/Fraxis.hpp>

#include "EnumDestinationMode.hpp"
#include "EnumOperation.hpp"

class QComboBox;
class QLabel;
class QPushButton;

namespace mdn {
namespace gui {

class OperationStrip : public QWidget
{
    Q_OBJECT
public:
    explicit OperationStrip(QWidget* parent = nullptr);

    // Called by OpsController when an operation session starts.
    void battlestations(Operation op);

    // Called by OpsController when the session ends/cancels.
    void reset();

    void enterActiveDivisionVisual();
    void leaveActiveDivisionVisual();

    // Get the value from division iterations spin box
    int divisionIterations() const;
    Fraxis divisionFraxis() const;

    void setOpsEnabled(bool enabled);

signals:
    void transposeClicked();
    void carryOverClicked();
    void carryPosClicked();
    void carryNegClicked();
    void operationClicked(Operation op);
    void cancelClicked();
    void propertiesClicked();

    // user clicked Divide while in ActiveDivision mode
    void divisionIterateClicked(int nIters);

    // user requested to stop active division (Cancel/Escape)
    void divisionStopRequested();

private slots:
    void divCycleFraxis();
    void divChooseFraxisX();
    void divChooseFraxisY();
    void divChooseFraxisAlternating();

private:

    // Private functions
    QToolButton* buttonFor(Operation op) const;
    void setOthersDisabledExcept(Operation op);
    void buttonEnableAndHighlight(QAbstractButton* btn, bool enable);

    // Private data
    QPushButton* m_btnCancel = nullptr;

    QToolButton* m_btnAdd = nullptr;
    QToolButton* m_btnSub = nullptr;
    QToolButton* m_btnMul = nullptr;
    QToolButton* m_btnDiv = nullptr;
    std::array<QToolButton*, 4> m_allOpButtons;

    QLabel* m_divIterLabel = nullptr;
    QSpinBox*  m_divIter = nullptr;
    // QLabel* m_divAlgoLabel = nullptr;

    Fraxis m_divFraxis{Fraxis::Default};
    QToolButton* m_divFraxisBtn{nullptr};
    QMenu* m_divFraxisMenu{nullptr};

    QToolButton* m_btnTranspose = nullptr;
    QToolButton* m_btnCarryOver = nullptr;
    QToolButton* m_btnCarryPos = nullptr;
    QToolButton* m_btnCarryNeg = nullptr;

    bool m_activeDivision{false};

private slots:
    void onAdd() {
        Log_Debug3("emit operationClicked(Add)");
        emit operationClicked(Operation::Add);
    }
    void onSub() {
        Log_Debug3("emit operationClicked(Subtract)");
        emit operationClicked(Operation::Subtract);
    }
    void onMul() {
        Log_Debug3("emit operationClicked(Multiply)");
        emit operationClicked(Operation::Multiply);
    }
    void onDiv();
    void onCancel();

};

} // end namespace gui
} // end namespace mdn
