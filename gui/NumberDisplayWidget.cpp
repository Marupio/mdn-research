#include "NumberDisplayWidget.hpp"

#include "Project.hpp"
#include "Selection.hpp"

mdn::gui::NumberDisplayWidget::NumberDisplayWidget(QWidget* parent)
    : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}


void mdn::gui::NumberDisplayWidget::setProject(Project* project) {
    m_project = project;
}


void mdn::gui::NumberDisplayWidget::setModel(const Mdn2d* model, Selection* sel) {
    m_model = model;
    m_selection = sel;
    if (m_selection) {
        m_cachedSel = m_selection->rect();
        m_cursorX = m_selection->cursor1().x();
        m_cursorY = m_selection->cursor1().y();
    }
    recalcGridGeometry();
    update();
}


void mdn::gui::NumberDisplayWidget::increaseFont() {
    adjustFontBy(+1);
}


void mdn::gui::NumberDisplayWidget::decreaseFont() {
    adjustFontBy(-1);
}


void mdn::gui::NumberDisplayWidget::resetFont() {
    setFontPointSize(11);
}


void mdn::gui::NumberDisplayWidget::setFontPointSize(int pt) {
    pt = std::clamp(pt, m_minPt, m_maxPt);
    if (m_theme.font.pointSize() == pt) return;
    m_theme.font.setPointSize(pt);
    recalcGridGeometry();
    // If you feed page step back to Project, do it here using visibleCols/Rows
    if (m_project) {
        const int pageCols = std::max(1, m_cols - 1);
        const int pageRows = std::max(1, m_rows - 1);
        m_project->setPageStep(pageCols, pageRows); // optional if you added this
    }
    update();
}


void mdn::gui::NumberDisplayWidget::onCursorChanged(mdn::Coord c) {
    m_cursorX = c.x(); m_cursorY = c.y();
    ensureCursorVisible();
    update();
}


void mdn::gui::NumberDisplayWidget::onSelectionChanged(const mdn::Rect& r) {
    m_cachedSel = r;
    update();
}


void mdn::gui::NumberDisplayWidget::onModelModified() {
    update();
}


void mdn::gui::NumberDisplayWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setRenderHint(QPainter::Antialiasing, false);

    const QRect widgetRect = rect();

    painter.setFont(m_theme.font);
    painter.setPen(m_theme.textPen);
    if (m_theme.background.style() != Qt::NoBrush) {
        painter.fillRect(widgetRect, m_theme.background);
    }

    AssertQ(
        (m_selection != nullptr) && (m_model != nullptr),
        "Model and Selection are not set"
    );

    const mdn::Rect& selRect = m_selection->rect();
    const mdn::Coord& anchor = m_selection->cursor0();
    const mdn::Coord& cursor = m_selection->cursor1();

    // Drawing per-cell:
    // 1) fill (cursor, anchor, selection)
    // 2) grid stroke
    // 3) digit
    // 4) origin overlay
    QPen gridPen = m_theme.gridPen;
    QPen textPen = m_theme.textPen;

    for (int vy = 0; vy < m_rows; ++vy) {
        for (int vx = 0; vx < m_cols; ++vx) {
            // Convert view cell (vx,vy) to model coordinate (x,y).
            // Screen row 0 is the top row. Model y+ goes up.
            const int modelX = m_viewOriginX + vx;
            const int modelY = m_viewOriginY + (m_rows - 1 - vy);

            const mdn::Coord xy(modelX, modelY);
            const int digit = static_cast<int>(m_model->getValue(xy));

            const QRect cell(vx * m_cellSize, vy * m_cellSize, m_cellSize, m_cellSize);

            // Apply fills in this order: selection, anchor, cursor
            if (selRect.contains(xy)) {
                painter.fillRect(cell, m_theme.selectionFill);
            }
            if (xy == anchor) {
                // Apply anchor cell style
                painter.fillRect(cell, m_theme.anchorFill);
            }
            if (xy == cursor) {
                // Apply cursor cell style
                painter.fillRect(cell, m_theme.cursorFill);
            }

            // Grid stroke
            painter.setPen(gridPen);
            painter.drawRect(cell);

            // Digit
            painter.setPen(textPen);
            painter.drawText(cell, Qt::AlignCenter, QString::number(digit));

            // Highlight origin
            if (xy == mdn::COORD_ORIGIN) {
                QPen op = m_theme.originPen;
                // keep inner rect inside the grid stroke
                const QRect inner = cell.adjusted(1, 1, -1, -1);
                painter.setPen(op);
                painter.drawRect(inner);

                // restore
                painter.setPen(gridPen);
            }
        }
    }

    // Axes (draw after cells so they sit on top)
    drawAxes(painter, widgetRect);
}


void mdn::gui::NumberDisplayWidget::keyPressEvent(QKeyEvent* e) {
    const Qt::KeyboardModifiers mods = e->modifiers();
    const bool shift  = mods.testFlag(Qt::ShiftModifier);
    const bool ctrl   = mods.testFlag(Qt::ControlModifier);
    const bool alt    = mods.testFlag(Qt::AltModifier);
    const bool extend = shift;

    // Zoom shortcuts: Ctrl+'+' / Ctrl+'-' / Ctrl+'0'
    if (ctrl) {
        if (e->key() == Qt::Key_Plus || e->key() == Qt::Key_Equal) {
            // '=' is '+' on many layouts
            increaseFont();
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_Minus) {
            decreaseFont();
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_0) {
            resetFont();
            e->accept();
            return;
        }
    }

    if (!m_selection || !m_project) {
        Log_Warn("NumberDisplayWidget key press event handling with no valid project or selection");
        QWidget::keyPressEvent(e);
        return;
    }

    // Navigation keys:
    switch (e->key()) {
        // ---- Arrow keys ----
        case Qt::Key_Up:
            if (ctrl) m_selection->cursorJumpUp(extend);
            else      m_selection->cursorUp(extend);
            e->accept(); break;

        case Qt::Key_Down:
            if (ctrl) m_selection->cursorJumpDn(extend);
            else      m_selection->cursorDn(extend);
            e->accept(); break;

        case Qt::Key_Left:
            if (ctrl) m_selection->cursorJumpLf(extend);
            else      m_selection->cursorLf(extend);
            e->accept(); break;

        case Qt::Key_Right:
            if (ctrl) m_selection->cursorJumpRt(extend);
            else      m_selection->cursorRt(extend);
            e->accept(); break;

        // ---- Paging ----
        case Qt::Key_PageUp:
            if (alt)  m_selection->cursorPageLf(extend);
            else      m_selection->cursorPageUp(extend);
            e->accept(); break;

        case Qt::Key_PageDown:
            if (alt)  m_selection->cursorPageRt(extend);
            else      m_selection->cursorPageDn(extend);
            e->accept(); break;

        // ---- Origin ----
        case Qt::Key_Home:
            m_selection->cursorOrigin(extend);
            e->accept(); break;

        // ---- Next / Prev by “entry” convention ----
        // Enter / Return: move along Y (down by default)
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (shift) m_selection->cursorPrevY(false);
            else       m_selection->cursorNextY(false);
            e->accept(); break;

        // Tab: move along X (right by default)
        case Qt::Key_Tab:
            if (shift) m_selection->cursorPrevX(false);
            else       m_selection->cursorNextX(false);
            e->accept(); break;

        // Shift+Tab comes in as Key_Backtab on some platforms
        case Qt::Key_Backtab:
            m_selection->cursorPrevX(false);
            e->accept(); break;

        case Qt::Key_Escape:
            emit focusDownRequested();
            break;

        default:
            QWidget::keyPressEvent(e);
            return;
    }

    // Ensure the caret stays visible and repaint immediately
    if (m_selection) {
        m_cursorX = m_selection->cursor1().x();
        m_cursorY = m_selection->cursor1().y();
        ensureCursorVisible();
    }
    e->accept();
    update();
}


void mdn::gui::NumberDisplayWidget::wheelEvent(QWheelEvent* e) {
    // Ctrl+Wheel zooms font
    if (e->modifiers().testFlag(Qt::ControlModifier)) {
        // 120 per notch
        const int numSteps = e->angleDelta().y() / 120;
        if (numSteps > 0) {
            increaseFont();
        } else if (numSteps < 0) {
            decreaseFont();
        }
        e->accept();
        return;
    }
    QWidget::wheelEvent(e);
}


void mdn::gui::NumberDisplayWidget::resizeEvent(QResizeEvent* e) {
    recalcGridGeometry();
    if (m_selection) {
        const int pageCols = std::max(1, m_cols - 1);
        const int pageRows = std::max(1, m_rows - 1);
        m_selection->setPageStep(pageCols, pageRows);
    }
    QWidget::resizeEvent(e);
    update();
}


void mdn::gui::NumberDisplayWidget::recalcGridGeometry() {
    // Size grid cells from current font so digits remain crisp
    QFontMetrics fm(m_theme.font);
    // Width of '0' is a good proxy in monospace; add small padding
    const int cw = fm.horizontalAdvance(QLatin1Char('0')) + 2;
    const int ch = fm.height() + 2;
    m_cellSize = std::max(cw, ch);

    m_cols = std::max(1, width()  / m_cellSize) + 1;
    m_rows = std::max(1, height() / m_cellSize) + 1;

    int pageCols = std::max(1, m_cols - 1);
    int pageRows = std::max(1, m_rows - 1);
    m_project->setPageStep(pageCols, pageRows);
}


void mdn::gui::NumberDisplayWidget::ensureCursorVisible() {
    // Horizontal: model x+ goes right (no inversion)
    if (m_cursorX < m_viewOriginX) {
        m_viewOriginX = m_cursorX;
    }
    if (m_cursorX >= m_viewOriginX + m_cols) {
        m_viewOriginX = m_cursorX - (m_cols - 1);
    }

    // Vertical: model y+ goes up, top of view shows higher model y
    if (m_cursorY >= m_viewOriginY + m_rows) {
        m_viewOriginY = m_cursorY - (m_rows - 1);
    }
    if (m_cursorY < m_viewOriginY) {
        m_viewOriginY = m_cursorY;
    }
}


void mdn::gui::NumberDisplayWidget::drawAxes(QPainter& painter, const QRect& widgetRect) {
    // X axis in view?
    if (m_viewOriginY <= 0 && 0 < m_viewOriginY + m_rows) {
        const int yIndex = m_viewOriginY + (m_rows - 1) - 0;
        const int yOriginPx = yIndex * m_cellSize;

        QPen axis = m_theme.axisPen;
        const int prevW = painter.pen().width();
        painter.setPen(axis);

        painter.drawLine(0, yOriginPx, widgetRect.width(), yOriginPx);

        axis.setWidth(prevW);
        painter.setPen(axis);
    }

    // Y axis in view?
    if (m_viewOriginX <= 0 && 0 < m_viewOriginX + m_cols) {
        const int xOriginPx = -m_viewOriginX * m_cellSize;

        QPen axis = m_theme.axisPen;
        const int prevW = painter.pen().width();
        painter.setPen(axis);

        painter.drawLine(xOriginPx, 0, xOriginPx, widgetRect.height());

        axis.setWidth(prevW);
        painter.setPen(axis);
    }

    if (hasFocus()) {
        QPen ring(QColor(70,120,255), 2);
        painter.setPen(ring);
        painter.drawRect(rect().adjusted(1,1,-2,-2));
    }
}


void mdn::gui::NumberDisplayWidget::adjustFontBy(int deltaPts) {
    setFontPointSize(fontPointSize() + deltaPts);
}
