#pragma once

#include <QWidget>
#include <QToolButton>
#include <QHBoxLayout>

namespace mdn {
namespace gui {

class OperationStrip : public QWidget {
    Q_OBJECT
public:
    explicit OperationStrip(QWidget* parent=nullptr);

    QToolButton *btnCancel, *btnAdd, *btnSub, *btnMul, *btnDiv, *btnNewTab;

private:
    static QToolButton* make(const QString& text) {
        auto b = new QToolButton;
        b->setText(text);
        b->setToolButtonStyle(Qt::ToolButtonTextOnly);
        return b;
    }
};

} // end namespace gui
} // end namespace mdn
