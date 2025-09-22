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

protected:
    void resizeEvent(QResizeEvent* e) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void applyText(const QString& text);
    void updateLineCountByWidth();

    QLabel*  m_label;
    QTimer   m_timer;
    QString  m_permanent;
    int      m_fontPx = 12;
};

} // namespace mdn::gui
