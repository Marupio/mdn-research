#include "HelpDialog.hpp"

#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTextCursor>
#include <QTextDocument>
#include <QFile>

HelpDialog::HelpDialog(QString title, QWidget* parent) :
    QDialog(parent),
    m_title(title)
{
    setWindowTitle(tr("Help"));
    resize(720, 540);

    m_view = new QTextEdit(this);
    m_view->setReadOnly(true);

    QFile f(":/help/" + title + ".md");
    // #ifdef QT_DEBUG
    //     QFile f(QCoreApplication::applicationDirPath() + "/help/overview.md");
    // #else
    //    QFile f(":/help/overview.md");
    // #endif
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QString md = QString::fromUtf8(f.readAll());
        m_view->setMarkdown(md); // Qt 5.14+ / Qt6
    } else {
        m_view->setHtml("<h2>Help</h2><p>Could not load help.</p>");
    }

    m_find = new QLineEdit(this);
    m_find->setPlaceholderText(tr("Find…"));
    connect(m_find, &QLineEdit::textChanged, this, [this](const QString& s){
        if (s.isEmpty()) return;
        auto cur = m_view->document()->find(s, m_view->textCursor());
        if (!cur.isNull()) m_view->setTextCursor(cur);
    });

    auto* closeBtn = new QPushButton(tr("Close"), this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    auto* lay = new QVBoxLayout(this);
    lay->addWidget(m_view);
    lay->addWidget(m_find);
    lay->addWidget(closeBtn);
    setLayout(lay);
}
