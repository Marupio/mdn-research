#include "CommandWidget.hpp"

#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QVBoxLayout>

mdn::gui::CommandWidget::CommandWidget(QWidget* parent)
    : QWidget(parent) {
    buildUi();
}


void mdn::gui::CommandWidget::appendLine(const QString& line) {
    if (m_history == nullptr) {
        return;
    }
    m_history->append(line);
    trimHistory();
}


void mdn::gui::CommandWidget::appendHtml(const QString& html) {
    if (m_history == nullptr) {
        return;
    }
    m_history->append(html);
    trimHistory();
}


QString mdn::gui::CommandWidget::currentInput() const {
    if (m_input == nullptr) {
        return QString();
    }
    return m_input->text();
}


void mdn::gui::CommandWidget::setCurrentInput(const QString& text) {
    if (m_input == nullptr) {
        return;
    }
    m_input->setText(text);
}


void mdn::gui::CommandWidget::clearInput() {
    if (m_input == nullptr) {
        return;
    }
    m_input->clear();
}


void mdn::gui::CommandWidget::setBusy(bool busy) {
    bool b = busy;
    if (m_input != nullptr) {
        m_input->setEnabled(!b);
    }
    if (m_submit != nullptr) {
        m_submit->setEnabled(!b);
    }
}


void mdn::gui::CommandWidget::setMaxHistoryLines(int n) {
    if (n <= 0) {
        return;
    }
    m_maxLines = n;
    trimHistory();
}


bool mdn::gui::CommandWidget::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_input) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Up) {
                recallPrev();
                return true;
            } else {
                if (ke->key() == Qt::Key_Down) {
                    recallNext();
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}


void mdn::gui::CommandWidget::onSubmit() {
    if (m_input == nullptr) {
        return;
    }
    QString text = m_input->text();
    if (text.trimmed().isEmpty()) {
        return;
    }
    m_inputHistory.append(text);
    m_histIndex = m_inputHistory.size();
    emit submitCommand(text);
    clearInput();
}


void mdn::gui::CommandWidget::onCopy() {
    if (m_history == nullptr) {
        return;
    }
    QTextCursor c = m_history->textCursor();
    bool hadSel = c.hasSelection();
    if (!hadSel) {
        m_history->selectAll();
    }
    m_history->copy();
    if (!hadSel) {
        c.clearSelection();
        m_history->setTextCursor(c);
    }
}


void mdn::gui::CommandWidget::onReturnPressed() {
    onSubmit();
}


void mdn::gui::CommandWidget::buildUi() {
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(6, 6, 6, 6);
    root->setSpacing(6);

    m_history = new QTextEdit(this);
    m_history->setReadOnly(true);
    m_history->setAcceptRichText(true);

    QWidget* inputRow = new QWidget(this);
    QHBoxLayout* row = new QHBoxLayout(inputRow);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(6);

    m_input = new QLineEdit(inputRow);
    m_input->installEventFilter(this);

    m_submit = new QPushButton(tr("Submit"), inputRow);
    m_copy = new QPushButton(tr("Copy"), inputRow);

    row->addWidget(m_input, 1);
    row->addWidget(m_submit);
    row->addWidget(m_copy);

    root->addWidget(m_history, 1);
    root->addWidget(inputRow);

    connect(m_submit, SIGNAL(clicked()), this, SLOT(onSubmit()));
    connect(m_copy, SIGNAL(clicked()), this, SLOT(onCopy()));
    connect(m_input, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()));
}


void mdn::gui::CommandWidget::trimHistory() {
    if (m_history == nullptr) {
        return;
    }
    if (m_maxLines <= 0) {
        return;
    }
    QTextDocument* doc = m_history->document();
    int blocks = doc->blockCount();
    if (blocks <= m_maxLines) {
        return;
    }
    QTextCursor cur(doc->firstBlock());
    int removeCount = blocks - m_maxLines;
    for (int i = 0; i < removeCount; ++i) {
        QTextBlock b = cur.block();
        if (!b.isValid()) {
            break;
        }
        cur.select(QTextCursor::BlockUnderCursor);
        cur.removeSelectedText();
        cur.deleteChar();
    }
}


void mdn::gui::CommandWidget::recallPrev() {
    if (m_inputHistory.isEmpty()) {
        return;
    }
    if (m_histIndex <= 0) {
        m_histIndex = 0;
    } else {
        m_histIndex = m_histIndex - 1;
    }
    setCurrentInput(m_inputHistory.value(m_histIndex));
}


void mdn::gui::CommandWidget::recallNext() {
    if (m_inputHistory.isEmpty()) {
        return;
    }
    if (m_histIndex >= m_inputHistory.size() - 1) {
        m_histIndex = m_inputHistory.size() - 1;
    } else {
        m_histIndex = m_histIndex + 1;
    }
    setCurrentInput(m_inputHistory.value(m_histIndex));
}
