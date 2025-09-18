#pragma once

#include <QWidget>
#include <QStringList>

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

    void setTabNames(const QStringList& names);
    void setActiveIndex(int indexA);
    void setRememberedB(int indexB);
    void setDestinationMode(DestinationSimple mode);

signals:
    void requestOperation(Operation op, int indexA, int indexB, DestinationSimple dest);
    void requestChangeB();

private slots:
    void onClickAdd();
    void onClickSub();
    void onClickMul();
    void onClickDiv();
    void onChangeB();
    void onDestChanged(int idx);

private:
    void emitOp(Operation op);

private:
    QPushButton* m_add{nullptr};
    QPushButton* m_sub{nullptr};
    QPushButton* m_mul{nullptr};
    QPushButton* m_div{nullptr};
    QLabel* m_labelA{nullptr};
    QComboBox* m_bPicker{nullptr};
    QComboBox* m_destPicker{nullptr};
    QStringList m_names;
    int m_indexA{0};
    int m_indexB{0};
};

} // end namespace gui
} // end namespace mdn
