#pragma once

#include <QWidget>
#include <QLabel>
#include <QTimer>

namespace mdn::gui
{

class StatusDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StatusDisplayWidget(QWidget* parent = nullptr);

    void setFontSize(int px);
    void showPermanentMessage(const QString& text);
    void showMessage(const QString& text, int timeoutMs);
    void clearMessage();

signals:
    void contentHeightChanged(int newHeight);

protected:
    void resizeEvent(QResizeEvent* e) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void applyText(const QString& text);
    void updateLineCountByWidth();
    void maybeEmitHeightChanged() {
        const int h = sizeHint().height();
        if (h != m_lastHintH) {
            m_lastHintH = h;
            emit contentHeightChanged(h);
        }
    }

    int      m_lastHintH = -1;
    QLabel*  m_label;
    QTimer   m_timer;
    QString  m_permanent;
    int      m_fontPx = 12;
};

} // namespace mdn::gui
