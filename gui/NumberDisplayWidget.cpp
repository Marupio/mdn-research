#include "NumberDisplayWidget.h"

#include "../library/Coord.h"
#include <QPainter>
#include <QKeyEvent>

NumberDisplayWidget::NumberDisplayWidget(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}

void NumberDisplayWidget::setModel(const mdn::Mdn2d* mdn) {
    m_model = mdn;
    update();
}

void NumberDisplayWidget::setViewCenter(int x, int y) {
    m_viewX = x;
    m_viewY = y;
    update();
}

void NumberDisplayWidget::moveCursor(int dx, int dy) {
    m_cursorX += dx;
    m_cursorY += dy;
    update();
}

void NumberDisplayWidget::paintEvent(QPaintEvent*) {
    if (!m_model) return;
    QPainter p(this);
    for (int y = 0; y < viewSize; ++y) {
        for (int x = 0; x < viewSize; ++x) {
            int gx = m_viewX + x;
            int gy = m_viewY + y;
            mdn::Coord gxy(m_viewX + x, m_viewY + y);
            mdn::Digit d = m_model->getValue(gxy);
            QRect cell(x * cellSize, y * cellSize, cellSize, cellSize);
            p.drawRect(cell);
            if (gx == m_cursorX && gy == m_cursorY) {
                p.fillRect(cell, Qt::yellow);
            }
            if (d != 0) {
                p.drawText(cell, Qt::AlignCenter, QString::number(d));
            }
        }
    }
}

void NumberDisplayWidget::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Left:  moveCursor(-1, 0); break;
        case Qt::Key_Right: moveCursor(1, 0); break;
        case Qt::Key_Up:    moveCursor(0, -1); break;
        case Qt::Key_Down:  moveCursor(0, 1); break;
    }
}
