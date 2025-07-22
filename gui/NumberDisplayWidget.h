#pragma once

#include <QWidget>
#include "Mdn2d.h"

class NumberDisplayWidget : public QWidget {
    Q_OBJECT

public:
    NumberDisplayWidget(QWidget* parent = nullptr);

    void setModel(const mdn::Mdn2d* mdn);
    void setViewCenter(int x, int y);
    void moveCursor(int dx, int dy);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    const mdn::Mdn2d* m_model = nullptr;
    int m_viewX = 0, m_viewY = 0;
    int m_cursorX = 0, m_cursorY = 0;
    static constexpr int cellSize = 20;
    static constexpr int viewSize = 32;
};
