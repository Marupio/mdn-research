#include "CellLineEdit.hpp"

mdn::gui::CellLineEdit::CellLineEdit(QWidget* parent)
    : QLineEdit(parent)
{
}

bool mdn::gui::CellLineEdit::focusNextPrevChild(bool)
{
    return false;
}
