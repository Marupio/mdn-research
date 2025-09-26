#pragma once

#include <array>

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

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

signals:
    void transposeClicked();
    void carryOverClicked();
    void carryPosClicked();
    void carryNegClicked();
    void operationClicked(Operation op);
    void cancelClicked();
    void propertiesClicked();

private:
    QPushButton* m_btnCancel = nullptr;

    QToolButton* m_btnAdd    = nullptr;
    QToolButton* m_btnSub    = nullptr;
    QToolButton* m_btnMul    = nullptr;
    QToolButton* m_btnDiv    = nullptr;
    std::array<QToolButton*, 4> m_allOpButtons;

    QToolButton* m_btnTranspose = nullptr;
    QToolButton* m_btnCarryOver = nullptr;
    QToolButton* m_btnCarryPos  = nullptr;
    QToolButton* m_btnCarryNeg  = nullptr;

    QToolButton* buttonFor(Operation op) const;
    void setOpsEnabled(bool enabled);
    void setOthersDisabledExcept(Operation op);

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
    void onDiv() {
        Log_Debug3("emit operationClicked(Divide)");
        emit operationClicked(Operation::Divide);
    }
    void onCancel() {
        Log_Debug3("emit cancelClicked()");
        emit cancelClicked();
    }
};

} // end namespace gui
} // end namespace mdn
