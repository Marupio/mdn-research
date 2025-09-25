#pragma once
#include <QDialog>

class QTextEdit;
class QLineEdit;

class HelpDialog : public QDialog {
    Q_OBJECT
public:
    explicit HelpDialog(QWidget* parent=nullptr);
private:
    QTextEdit*  m_view;
    QLineEdit*  m_find;
};
