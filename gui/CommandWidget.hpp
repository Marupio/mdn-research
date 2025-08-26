#pragma once

#include <QWidget>
#include <QStringList>

class QTextEdit;
class QLineEdit;
class QPushButton;

namespace mdn {
namespace gui {

class CommandWidget : public QWidget {
    Q_OBJECT

public:
    explicit CommandWidget(QWidget* parent = nullptr);

    void appendLine(const QString& line);
    void appendHtml(const QString& html);

    QString currentInput() const;
    void setCurrentInput(const QString& text);
    void clearInput();

    void setBusy(bool busy);
    void setMaxHistoryLines(int n);

signals:
    void submitCommand(const QString& text);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private slots:
    void onSubmit();
    void onCopy();
    void onReturnPressed();

private:
    void buildUi();
    void trimHistory();
    void recallPrev();
    void recallNext();

private:
    QTextEdit* m_history{nullptr};
    QLineEdit* m_input{nullptr};
    QPushButton* m_submit{nullptr};
    QPushButton* m_copy{nullptr};

    QStringList m_inputHistory;
    int m_histIndex{-1};
    int m_maxLines{2000};
};

} // end namespace gui
} // end namespace mdn
