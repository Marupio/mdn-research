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
    void activate(Operation op);

    // Called by OpsController when the session ends/cancels.
    void reset();

signals:
    // user clicked an op button
    void operationClicked(Operation op);
    // user clicked Cancel
    void cancelClicked();

private:
    QPushButton* m_btnCancel = nullptr;
    QPushButton* m_btnAdd    = nullptr;
    QPushButton* m_btnSub    = nullptr;
    QPushButton* m_btnMul    = nullptr;
    QPushButton* m_btnDiv    = nullptr;

    // exclusive group for op buttons
    QButtonGroup* m_group    = nullptr;

    QPushButton* buttonFor(Operation op) const;
    void setOpsEnabled(bool enabled);
    void setOthersDisabledExcept(Operation op);

private slots:
    void onAdd() { emit operationClicked(Operation::Add); }
    void onSub() { emit operationClicked(Operation::Subtract); }
    void onMul() { emit operationClicked(Operation::Multiply); }
    void onDiv() { emit operationClicked(Operation::Divide); }
    void onCancel() { emit cancelClicked(); }
};

} // end namespace gui
} // end namespace mdn
