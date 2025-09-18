#include "OperationStrip.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

mdn::gui::OperationStrip::OperationStrip(QWidget* parent) :
    QWidget(parent)
{
    auto row = new QHBoxLayout(this);
    row->setContentsMargins(0,0,0,0);
    btnCancel  = make("Cancel");   btnCancel->setEnabled(false);
    btnAdd     = make("+");
    btnSub     = make("–");
    btnMul     = make("×");
    btnDiv     = make("÷");
    btnNewTab  = make("NewTab");

    row->addWidget(btnCancel);
    row->addSpacing(8);
    row->addWidget(btnAdd);
    row->addWidget(btnSub);
    row->addWidget(btnMul);
    row->addWidget(btnDiv);
    row->addSpacing(8);
    row->addWidget(btnNewTab);
    setLayout(row);

    // Give them object names if you like for styling
    btnCancel->setObjectName("opCancel");
    btnNewTab->setObjectName("opNewTab");
}
