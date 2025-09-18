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

    void setCancelEnabled(bool on) { btnCancel->setEnabled(on); }

signals:
    void requestOperation(Operation op);   // user clicked +/−/×/÷
    void requestCancel();                  // clicked [Cancel]
    void requestNewTab();                  // clicked [NewTab]

private slots:
    // void onClickAdd();
    // void onClickSub();
    // void onClickMul();
    // void onClickDiv();

private:
    void emitOp(Operation op);

    static QToolButton* makeButton(const QString& text);

    void makeOpButton(QHBoxLayout* row, Operation op);

    QPushButton* m_add{nullptr};
    QPushButton* m_sub{nullptr};
    QPushButton* m_mul{nullptr};
    QPushButton* m_div{nullptr};

    QToolButton* btnCancel = nullptr;
    QToolButton* btnNewTab = nullptr;
};

} // end namespace gui
} // end namespace mdn
