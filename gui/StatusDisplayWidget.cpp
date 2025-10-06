#include "StatusDisplayWidget.hpp"

#include <QHBoxLayout>
#include <QFont>
#include <QFontMetrics>
#include <QResizeEvent>
#include <QPalette>

#include <mdn/Logger.hpp>

mdn::gui::StatusDisplayWidget::StatusDisplayWidget(QWidget* parent)
    : QWidget(parent),
      m_label(new QLabel(this))
{
    setObjectName("statusDisplay");
    setAttribute(Qt::WA_StyledBackground, true);

    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(8, 4, 8, 4);
    lay->addWidget(m_label);

    m_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_label->setWordWrap(true);
    m_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    // Subtle rounded outline, no fill (blends with window)
    // const auto mid = palette().color(QPalette::Mid).name();
    // // setStyleSheet(QString(
    // //     "StatusDisplayWidget { background: transparent; border: 1px solid %1; border-radius: 8px; } "
    // //     "StatusDisplayWidget QLabel { background: transparent; }"
    // // ).arg(mid));

    // // Border using palette so it shows in both light/dark themes
    // setStyleSheet(QString(
    //     "#statusDisplay {"
    //     "  background: transparent;"
    //     "  border: 1px solid palette(%1);"
    //     "  border-radius: 8px;"
    //     "}"
    //     "#statusDisplay QLabel { background: transparent; }"
    // ).arg(mid));

    setStyleSheet(QString(
        "#statusDisplay {"
        "  background: transparent;"
        "  border: 1px solid palette(Mid);"
        "  border-radius: 8px;"
        "}"
        "#statusDisplay QLabel { background: transparent; }"
    ));


    connect(&m_timer, &QTimer::timeout, this, [this]() {
        m_timer.stop();
        applyText(m_permanent);
    });

    setFontSize(m_fontPx);
    showPermanentMessage(QString());
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setMinimumHeight(sizeHint().height());
}


void mdn::gui::StatusDisplayWidget::setFontSize(int px) {
    m_fontPx = px > 0 ? px : 12;
    QFont f = m_label->font();
    f.setPixelSize(m_fontPx);
    m_label->setFont(f);
    updateLineCountByWidth();
    updateGeometry();
    maybeEmitHeightChanged();
}


void mdn::gui::StatusDisplayWidget::showPermanentMessage(const QString& text)
{
    m_permanent = text;
    if (!m_timer.isActive())
    {
        applyText(m_permanent);
    }
}


void mdn::gui::StatusDisplayWidget::showMessage(const QString& text, int timeoutMs)
{
    if (timeoutMs <= 0)
    {
        m_timer.stop();
        showPermanentMessage(text);
        return;
    }

    applyText(text);
    m_timer.start(timeoutMs);
}


void mdn::gui::StatusDisplayWidget::clearMessage() {
    showPermanentMessage("");
}


void mdn::gui::StatusDisplayWidget::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    updateLineCountByWidth();
    updateGeometry();
}


QSize mdn::gui::StatusDisplayWidget::sizeHint() const
{
    const QFontMetrics fm(m_label->font());
    const int lines = m_label->wordWrap() ? 2 : 1;
    const int h = fm.height() * lines + 8; // 8 ~= top+bottom margins in layout
    return { QWidget::sizeHint().width(), h };
}


QSize mdn::gui::StatusDisplayWidget::minimumSizeHint() const
{
    const QFontMetrics fm(m_label->font());
    const int h = fm.height() + 8;
    return { 100, h }; // 100px min width keeps it from collapsing to nothing
}


void mdn::gui::StatusDisplayWidget::applyText(const QString& text)
{
    m_label->setText(text);
    updateLineCountByWidth();
    updateGeometry();
}


void mdn::gui::StatusDisplayWidget::updateLineCountByWidth()
{
    // Decide 1 or 2 lines based on whether 80 avg-width chars fit in current width
    const QFontMetrics fm(m_label->font());
    const int avgChar = fm.averageCharWidth();
    const int targetChars = 80;
    const int neededPx = avgChar * targetChars;

    const int w = m_label->width() > 0 ? m_label->width() : width();
    const bool oneLineFits = (w >= neededPx);

    m_label->setWordWrap(!oneLineFits);
}
