#pragma once

#include <QHBoxLayout>
#include <QStringList>
#include <QToolButton>
#include <QWidget>

#include "EnumDestinationMode.hpp"
#include "EnumOperation.hpp"

class QComboBox;
class QLabel;
class QPushButton;

namespace mdn {
namespace gui {


class OperationStrip : public QWidget {
    Q_OBJECT

public:
    explicit OperationStrip(QWidget* parent = nullptr);

    void setCancelEnabled(bool on) { m_btnCancel->setEnabled(on); }

private slots:
    // void onClickAdd();
    // void onClickSub();
    // void onClickMul();
    // void onClickDiv();

signals:
    void requestOperation(Operation op);
    void requestCancel();
    void requestNewTab();

private:
    void emitOp(Operation op);

    // QPushButton* m_add{nullptr};
    // QPushButton* m_sub{nullptr};
    // QPushButton* m_mul{nullptr};
    // QPushButton* m_div{nullptr};

    QToolButton* m_btnCancel = nullptr;
    QToolButton* m_btnNewTab = nullptr;

    static QToolButton* makeButton(const QString& text);
    void addOpButton(QHBoxLayout* row, Operation op);
};

} // end namespace gui
} // end namespace mdn
