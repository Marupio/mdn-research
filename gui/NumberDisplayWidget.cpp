#include "NumberDisplayWidget.hpp"

#include <QFontMetrics>
#include <QKeyEvent>
#include <QPainter>
#include <QResizeEvent>

#include "../library/Coord.hpp"
#include "Project.hpp"


NumberDisplayWidget::NumberDisplayWidget(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}


void NumberDisplayWidget::setProject(mdn::Project* proj) {
    m_project = proj;
    update();
}


void NumberDisplayWidget::setModel(mdn::Mdn2d* mdn, mdn::Selection* sel) {
    m_model = mdn;
    m_selection = sel;
    update();
}


void NumberDisplayWidget::setViewCenter(int x, int y) {
    m_viewOriginX = x;
    m_viewOriginY = y;
    update();
}


// void NumberDisplayWidget::moveCursor(int dx, int dy) {
//     m_cursorX += dx;
//     m_cursorY += dy;
//     update();
// }


void NumberDisplayWidget::keyPressEvent(QKeyEvent* e) {
    if (!m_selection || !m_project) {
        Log_Warn("NumberDisplayWidget key press event handling with no valid project or selection");
        QWidget::keyPressEvent(e);
        return;
    }
    const Qt::KeyboardModifiers mods = e->modifiers();
    const bool shift  = mods.testFlag(Qt::ShiftModifier);
    const bool ctrl   = mods.testFlag(Qt::ControlModifier);
    const bool alt    = mods.testFlag(Qt::AltModifier);
    const bool extend = shift;

    // Navigation keys:
    switch (e->key()) {
        // ---- Arrow keys ----
        case Qt::Key_Up:
            if (ctrl) m_selection->cursorJumpUp(extend);
            else      m_selection->cursorUp(extend);
            e->accept(); return;

        case Qt::Key_Down:
            if (ctrl) m_selection->cursorJumpDn(extend);
            else      m_selection->cursorDn(extend);
            e->accept(); return;

        case Qt::Key_Left:
            if (ctrl) m_selection->cursorJumpLf(extend);
            else      m_selection->cursorLf(extend);
            e->accept(); return;

        case Qt::Key_Right:
            if (ctrl) m_selection->cursorJumpRt(extend);
            else      m_selection->cursorRt(extend);
            e->accept(); return;

        // ---- Paging ----
        case Qt::Key_PageUp:
            if (alt)  m_selection->cursorPageLf(extend);
            else      m_selection->cursorPageUp(extend);
            e->accept(); return;

        case Qt::Key_PageDown:
            if (alt)  m_selection->cursorPageRt(extend);
            else      m_selection->cursorPageDn(extend);
            e->accept(); return;

        // ---- Origin ----
        case Qt::Key_Home:
            m_selection->cursorOrigin(extend);
            e->accept(); return;

        // ---- Next / Prev by “entry” convention ----
        // Enter / Return: move along Y (down by default)
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (shift) m_selection->cursorPrevY(false);
            else       m_selection->cursorNextY(false);
            e->accept(); return;

        // Tab: move along X (right by default)
        case Qt::Key_Tab:
            if (shift) m_selection->cursorPrevX(false);
            else       m_selection->cursorNextX(false);
            e->accept(); return;

        // Shift+Tab comes in as Key_Backtab on some platforms
        case Qt::Key_Backtab:
            m_selection->cursorPrevX(false);
            e->accept(); return;

        default:
            break;
    }

    // Fallback for anything we don't handle
    QWidget::keyPressEvent(e);
}


void NumberDisplayWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    QRect widgetRect = this->rect();  // Get the full widget rectangle
    painter.setFont(QFont("Courier", 10)); // monospaced font
    painter.setPen(m_defaultColors_gridLines);
    AssertQ(
        (m_selection != nullptr) && (m_model != nullptr),
        "Model and Selection are not set"
    );

    const mdn::Rect& r = m_selection->rect();
    const mdn::Coord& c0 = m_selection->cursor0();
    const mdn::Coord& c1 = m_selection->cursor1();
    for (int y = 0; y < m_rows; ++y) {
        for (int x = 0; x < m_cols; ++x) {
            mdn::Coord xy(m_viewOriginX + x, m_viewOriginY + y);
            const int digit = static_cast<int>(m_model->getValue(xy));

            QRect cell(x * m_cellSize, y * m_cellSize, m_cellSize, m_cellSize);
            if (xy == c1) {
                // Apply cursor cell style
                painter.fillRect(cell, Qt::yellow);
            } else if (xy == c0) {
                // Apply anchor cell style
                painter.fillRect(cell, Qt::lightGray);
            } else if (r.contains(xy)) {
                // Apply selection rectangle cell style
                // ???
            }
            painter.drawRect(cell);

            QString text = QString::number(digit);
            painter.drawText(cell, Qt::AlignCenter, text);

            // Highlight origin
            if (xy == mdn::COORD_ORIGIN) {
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


void NumberDisplayWidget::resizeEvent(QResizeEvent* e) {
    recalcGridGeometry();
    QWidget::resizeEvent(e);
    update();
}


void NumberDisplayWidget::ensureCursorVisible() {
    // left
    int cursorX = m_selection->cursor1().x();
    int cursorY = m_selection->cursor1().y();
    if (cursorX < m_viewOriginX) {
        m_viewOriginX = cursorX;
    }
    // right
    if (cursorX >= m_viewOriginX + m_cols) {
        m_viewOriginX = cursorX - (m_cols - 1);
    }
    // top
    if (cursorY < m_viewOriginY) {
        m_viewOriginY = cursorY;
    }
    // bottom
    if (cursorY >= m_viewOriginY + m_rows) {
        m_viewOriginY = cursorY - (m_rows - 1);
    }
    ensureCursorVisible();
    update();
}


void NumberDisplayWidget::recalcGridGeometry() {
    // Option A: keep m_cellSize fixed (20 logical px)
    // Option B (nicer): derive from current font to keep things crisp
    QFontMetrics fm(font());
    int cw = fm.horizontalAdvance('0');
    int ch = fm.height();
    m_cellSize = std::max({m_cellSize, cw, ch}); // keep at least current, or pin to cw/ch if you prefer

    const int w = width();
    const int h = height();
    m_cols = std::max(1, w / m_cellSize) + 1;
    m_rows = std::max(1, h / m_cellSize) + 1;

    int pageCols = std::max(1, m_cols - 1);
    int pageRows = std::max(1, m_rows - 1);
    m_project->setPageStep(pageCols, pageRows);
}

