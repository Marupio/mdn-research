#include "NumberDisplayWidget.hpp"

#include <QFontMetrics>
#include <QKeyEvent>
#include <QPainter>
#include <QResizeEvent>

#include "../library/Coord.hpp"


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
    m_viewOriginX = x;
    m_viewOriginY = y;
    update();
}


void NumberDisplayWidget::moveCursor(int dx, int dy) {
    m_cursorX += dx;
    m_cursorY += dy;
    update();
}


void NumberDisplayWidget::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
        case Qt::Key_Up:    emit cursorMoveRequested(0, -1, e->modifiers()); break;
        case Qt::Key_Down:  emit cursorMoveRequested(0,  1, e->modifiers()); break;
        case Qt::Key_Left:  emit cursorMoveRequested(-1, 0, e->modifiers()); break;
        case Qt::Key_Right: emit cursorMoveRequested( 1, 0, e->modifiers()); break;
        default: QWidget::keyPressEvent(e); return;
    }
}


void NumberDisplayWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    QRect widgetRect = this->rect();  // Get the full widget rectangle
    painter.setFont(QFont("Courier", 10)); // monospaced font
    painter.setPen(m_defaultColors_gridLines);

    for (int y = 0; y < m_rows; ++y) {
        for (int x = 0; x < m_cols; ++x) {
            int currentX = m_viewOriginX + x;
            int currentY = m_viewOriginY + y;
            const int digit = m_model ? m_model->getValue({currentX, currentY}) : 0;

            QRect cell(x * m_cellSize, y * m_cellSize, m_cellSize, m_cellSize);
            if (currentX == m_cursorX && currentY == m_cursorY) {
                painter.fillRect(cell, Qt::yellow);
            }
            painter.drawRect(cell);

            QString text = QString::number(digit);
            painter.drawText(cell, Qt::AlignCenter, text);

            // Highlight origin
            if (currentX == 0 && currentY == 0) {
                painter.setPen(Qt::red);
                painter.drawRect(cell.adjusted(1, 1, -1, -1));
                painter.setPen(m_defaultColors_gridLines);
            }
        }
    }

    // Draw axes if origin is in view
    if (m_viewOriginY <= 0 && 0 < m_viewOriginY + m_rows) {
        int origWidth = painter.pen().width();
        QPen axisPen(Qt::gray);
        axisPen.setWidth(3);
        painter.setPen(axisPen);

        int yOriginPixel = -m_viewOriginY * m_cellSize;
        painter.drawLine(0, yOriginPixel, widgetRect.width(), yOriginPixel); // Horizontal x-axis

        axisPen.setWidth(origWidth);
        axisPen.setColor(m_defaultColors_gridLines);
        painter.setPen(axisPen);
    }

    if (m_viewOriginX <= 0 && 0 < m_viewOriginX + m_cols) {
        int origWidth = painter.pen().width();
        QPen axisPen(Qt::gray);
        axisPen.setWidth(3);
        painter.setPen(axisPen);

        int xOriginPixel = -m_viewOriginX * m_cellSize;
        painter.drawLine(xOriginPixel, 0, xOriginPixel, widgetRect.height()); // Vertical y-axis

        axisPen.setWidth(origWidth);
        axisPen.setColor(m_defaultColors_gridLines);
        painter.setPen(axisPen);
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


void NumberDisplayWidget::resizeEvent(QResizeEvent* e) {
    recalcGridGeometry();
    QWidget::resizeEvent(e);
    update();
}


void NumberDisplayWidget::ensureCursorVisible() {
    // left
    if (m_cursorX < m_viewOriginX) {
        m_viewOriginX = m_cursorX;
    }
    // right
    if (m_cursorX >= m_viewOriginX + m_cols) {
        m_viewOriginX = m_cursorX - (m_cols - 1);
    }
    // top
    if (m_cursorY < m_viewOriginY) {
        m_viewOriginY = m_cursorY;
    }
    // bottom
    if (m_cursorY >= m_viewOriginY + m_rows) {
        m_viewOriginY = m_cursorY - (m_rows - 1);
    }
}


// void NumberDisplayWidget::moveCursor(int dx, int dy) {
//     m_cursorX += dx;
//     m_cursorY += dy;
//     ensureCursorVisible();
//     update();
// }


void NumberDisplayWidget::recalcGridGeometry() {
    // Option A: keep m_cellSize fixed (20 logical px)
    // Option B (nicer): derive from current font to keep things crisp
    QFontMetrics fm(font());
    int cw = fm.horizontalAdvance('0');
    int ch = fm.height();
    m_cellSize = std::max({m_cellSize, cw, ch}); // keep at least current, or pin to cw/ch if you prefer

    const int w = width();
    const int h = height();
    m_cols = std::max(1, w / m_cellSize);
    m_rows = std::max(1, h / m_cellSize);
}

