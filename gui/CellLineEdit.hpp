#pragma once
#include <QLineEdit>

namespace mdn {
namespace gui {

class CellLineEdit final : public QLineEdit {
    Q_OBJECT
public:
    explicit CellLineEdit(QWidget* parent = nullptr);

protected:
    bool focusNextPrevChild(bool next) override;
};

} // end namespace gui
} // end namespace mdn