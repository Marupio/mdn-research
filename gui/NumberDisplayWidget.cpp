#include "NumberDisplayWidget.hpp"

#include <QFontMetrics>
#include <QGuiApplication>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>

#include "CellLineEdit.hpp"
#include "GuiTools.hpp"
#include "Project.hpp"
#include "Selection.hpp"
#include "Tools.hpp"

constexpr double mdn::gui::NumberDisplayWidget::kEdgeGuardFrac;


std::string mdn::gui::NumberDisplayWidget::EditModeToString(EditMode m) {
    switch (m) {
        case EditMode::Overwrite:
            return "Overwrite";
        case EditMode::Add:
            return "Add";
        case EditMode::Subtract:
            return "Subtract";
        default:
            return "Unknown";
    }
}


mdn::gui::NumberDisplayWidget::NumberDisplayWidget(QWidget* parent)
    : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}


void mdn::gui::NumberDisplayWidget::setProject(Project* project) {
    m_project = project;
}


void mdn::gui::NumberDisplayWidget::setModel(Mdn2d* model, Selection* sel) {
    Log_Debug2_H("model=" << model->name());
    m_model = model;
    m_selection = sel;
    if (m_selection) {
        m_cursorX = m_selection->cursor1().x();
        m_cursorY = m_selection->cursor1().y();
    }
    recalcGridGeometry();
    centreViewOnOrigin();
    update();
    Log_Debug2_T("");
}


void mdn::gui::NumberDisplayWidget::increaseFont() {
    adjustFontBy(+1);
}


void mdn::gui::NumberDisplayWidget::decreaseFont() {
    adjustFontBy(-1);
}


void mdn::gui::NumberDisplayWidget::resetFont() {
    emit requestFontSizeChange(11);
    // setFontPointSize(11);
}


void mdn::gui::NumberDisplayWidget::setFontPointSize(int pt) {
    captureCursorFractions();
    pt = std::clamp(pt, m_minPt, m_maxPt);
    if (m_theme.font.pointSize() == pt) return;
    m_theme.font.setPointSize(pt);
    if (m_cellEditor) {
        m_cellEditor->setFont(m_theme.font);
        positionCellEditor();
    }
    recalcGridGeometry();
    if (m_project) {
        const int pageCols = std::max(1, (m_cols - 1) / 3);
        const int pageRows = std::max(1, (m_rows - 1) / 3);
        m_project->setPageStep(pageCols, pageRows);
    }
    recalcGridGeometry();
    restoreCursorFractions();
    update();
}


void mdn::gui::NumberDisplayWidget::cancelCellEdit()
{
    Log_Debug3_H("");
    if (m_cellEditor) {
        m_cellEditor->hide();
    }
    m_editing = false;
    update();
    setFocus(Qt::OtherFocusReason);
    Log_Debug3_T("");
}


void mdn::gui::NumberDisplayWidget::onCursorChanged(mdn::Coord c) {
    m_cursorX = c.x(); m_cursorY = c.y();
    ensureCursorVisible();
    emit statusCursorChanged(m_cursorX, m_cursorY);
    update();
}


void mdn::gui::NumberDisplayWidget::onSelectionChanged(const mdn::Rect& r) {
    emit statusSelectionChanged(*m_selection);
    update();
}


void mdn::gui::NumberDisplayWidget::onModelModified() {
    update();
}


void mdn::gui::NumberDisplayWidget::setEditMode(EditMode m) {
    if (m_mode == m) {
        return;
    }
    m_mode = m;
    emit editModeChanged(m_mode);
    update();
}


void mdn::gui::NumberDisplayWidget::setHighlightRole(HighlightRole r)
{
    if (m_highlightRole != r)
    {
        m_highlightRole = r;
        update();
    }
}


void mdn::gui::NumberDisplayWidget::paintEvent(QPaintEvent* event) {
    if (m_highlightRole != HighlightRole::None)
    {
        QColor c;
        switch (m_highlightRole)
        {
            case HighlightRole::Peek:
                c = QColor(255, 235, 59, 40); // soft amber @ ~16% opacity
                break;
            default:
                c = QColor(0, 0, 0, 0);
                break;
        }

        if (c.alpha() > 0)
        {
            QPainter p(this);
            p.setPen(Qt::NoPen);
            p.setBrush(c);
            p.drawRect(rect());
        }
    }
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
    QPen nzGridPen = m_theme.nzGridPen;
    QPen nzTextPen = m_theme.nzTextPen;
    QPen gridPen = m_theme.gridPen;
    QPen textPen = m_theme.textPen;

    // Actual window bounds:
    //  m_viewOriginY + m_rows - 1
    //  m_viewOriginY + m_rows - 1 - (m_rows-1) = m_viewOriginY
    //  y range = [m_viewOriginY .. m_viewOriginY+m_rows-1]
    //  x range = [m_viewOriginX .. m_viewOriginX + m_cols-1]
    m_viewBounds.set(
        m_viewOriginX,              m_viewOriginY,
        m_viewOriginX + m_cols-1,   m_viewOriginY+m_rows-1,
        false // No need to fix ordering
    );

// Non-blocking fetch: try; if writer active, reuse cache and queue a repaint
VecVecDigit freshRows;
bool got = false;
if (m_model) {
    // NEW: model side should provide a try-get that returns false if a writer holds the lock
    got = m_model->tryGetAreaRows(m_viewBounds, freshRows);
}

    const VecVecDigit* rowsPtr = nullptr;
    if (got) {
        m_cachedRows = std::move(freshRows);
        m_haveCacheRows  = true;
        rowsPtr      = &m_cachedRows;
    } else {
        if (m_haveCacheRows) {
            rowsPtr = &m_cachedRows;
            // Try again soon; don't block the UI now
            QTimer::singleShot(0, this, [this]{ update(); });
        } else {
            // First frame and model busy: synthesize zeros so we can still draw the grid
            m_cachedRows.assign(std::max(1, m_rows), VecDigit(std::max(1, m_cols), Digit(0)));
            m_haveCacheRows = true;
            rowsPtr     = &m_cachedRows;
            QTimer::singleShot(0, this, [this]{ update(); });
        }
    }

    Rect modelBounds;
    if (!m_model->tryGetBounds(modelBounds)) {
        if (m_haveCacheBounds) {
            modelBounds = m_boundsCache;
        } else {
            // Log_Warn("Failed to acquire bounds and no cache");
        }
    } else {
        m_boundsCache = modelBounds;
        m_haveCacheBounds = true;
    }
    const Rect viewIbounds = Rect::Intersection(m_viewBounds, modelBounds);
    // int nzRowStart = -1;
    // int nzRowEnd = -1;
    // int nzColStart = -1;
    // int nzColEnd = -1;
    // if (!viewIbounds.empty()) {
    //     nzRowStart = viewIbounds.min().y() - m_viewBounds.min().y();
    //     nzRowEnd = m_viewBounds.height() - (m_viewBounds.max().y() - viewIbounds.max().y());
    //     nzColStart = viewIbounds.min().x() - m_viewBounds.min().x();
    //     nzColEnd = m_viewBounds.width() - (m_viewBounds.max().x() - viewIbounds.max().x());
    // }
    //
    // // To keep things confusing, we have vy = 0 at the top of the view window, increasing downwards
    // // And rowI = 0 at the bottom of the view window, increasing upwards
    // //      +---------------------------------------------------+ vy=0    rowI=m_rows-1
    // //      |                 View window                       |
    // //      |              +--------------------------------+...|...> nzRowEnd (rowI units)
    // //      |              |                                |   |
    // //      |              |      Non-zero bounds           |   |
    // //      |              |                                |   |
    // //      |              +--------------------------------+...|...> nzRowStart (rowI units)
    // //      |--nzColStart->.                   ---nzColEnd->.   |
    // //      +---------------------------------------------------+ vy=m_rows-1    rowI=0
    // //
    //
    // // bool nonZero = false;
    for (int vy = 0; vy < m_rows; ++vy) {

        int rowI = m_rows - 1 - vy;
        const VecDigit& currentRow = rowsPtr->operator[](rowI);

        for (int vx = 0; vx < m_cols; ++vx) {
            // Convert view cell (vx,vy) to model coordinate (x,y).
            // Screen row 0 is the top row. Model y+ goes up.
            const int modelX = m_viewOriginX + vx;
            const int modelY = m_viewOriginY + rowI;

            const Coord xy(modelX, modelY);
            const int digit = static_cast<int>(currentRow[vx]);

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

            QPen restorePen;
            if (viewIbounds.contains(xy)) {
                // Non-zero grid stroke
                painter.setPen(nzGridPen);
                painter.drawRect(cell);

                // Non-zero digit
                painter.setPen(nzTextPen);
                painter.drawText(cell, Qt::AlignCenter, QString::number(digit));
                restorePen = nzGridPen;
            } else {
                // Zero grid stroke
                painter.setPen(gridPen);
                painter.drawRect(cell);

                // Zero digit
                painter.setPen(textPen);
                painter.drawText(cell, Qt::AlignCenter, QString::number(digit));
                restorePen = gridPen;
            }

            // Highlight origin
            if (xy == mdn::COORD_ORIGIN) {
                QPen op = m_theme.originPen;
                // keep inner rect inside the grid stroke
                const QRect inner = cell.adjusted(1, 1, -1, -1);
                painter.setPen(op);
                painter.drawRect(inner);

                // restore
                painter.setPen(restorePen);
            }
        }
    }

    // Axes (draw after cells so they sit on top)
    drawAxes(painter, widgetRect);
}


bool mdn::gui::NumberDisplayWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_cellEditor) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* ke = static_cast<QKeyEvent*>(event);

            const bool ctrl = (ke->modifiers() & Qt::ControlModifier) != 0;
            const bool meta = (ke->modifiers() & Qt::MetaModifier) != 0;
            const bool shift = (ke->modifiers() & Qt::ShiftModifier) != 0;

            switch (ke->key()) {
                case Qt::Key_PageUp: {
                    if (!ctrl) {
                        break;
                    }
                    if (shift) {
                        Q_EMIT requestMoveTabLeft();
                    } else {
                        Q_EMIT requestSelectPrevTab();
                    }
                    ke->accept();
                    return true;
                }
                case Qt::Key_PageDown: {
                    if (!ctrl) {
                        break;
                    }
                    if (shift) {
                        Q_EMIT requestMoveTabRight();
                    } else {
                        Q_EMIT requestSelectNextTab();
                    }
                    ke->accept();
                    return true;
                }
                case Qt::Key_A: {
                    if (!ctrl && !meta) {
                        break;
                    }
                    cancelCellEdit();
                    setFocus(Qt::OtherFocusReason);
                    selectAllBounds();
                    ke->accept();
                    return true;
                }
                case Qt::Key_Return:
                case Qt::Key_Enter: {
                    if (shift) {
                        commitCellEdit(SubmitMove::ShiftEnter, true); // stay inside
                    } else {
                        commitCellEdit(SubmitMove::Enter, true); // stay inside
                    }
                    setFocus(Qt::OtherFocusReason);
                    ke->accept();
                    return true;
                }
                case Qt::Key_Tab: {
                    commitCellEdit(SubmitMove::Tab, true);
                    setFocus(Qt::OtherFocusReason);
                    ke->accept();
                    return true;
                }
                case Qt::Key_Backtab: {
                    commitCellEdit(SubmitMove::ShiftTab, true); // stay inside
                    setFocus(Qt::OtherFocusReason);
                    ke->accept();
                    return true;
                }
                case Qt::Key_Escape: {
                    cancelCellEdit();
                    setFocus(Qt::OtherFocusReason);
                    ke->accept();
                    return true;
                }
                case Qt::Key_Up: {
                    commitCellEdit(SubmitMove::ShiftEnter, false); // do not stay inside
                    setFocus(Qt::OtherFocusReason);
                    ke->accept();
                    return true;
                }
                case Qt::Key_Down: {
                    commitCellEdit(SubmitMove::Enter, false); // do not stay inside
                    setFocus(Qt::OtherFocusReason);
                    ke->accept();
                    return true;
                }
                case Qt::Key_Right: {
                    commitCellEdit(SubmitMove::Tab, false); // do not stay inside
                    setFocus(Qt::OtherFocusReason);
                    ke->accept();
                    return true;
                }
                case Qt::Key_Left: {
                    commitCellEdit(SubmitMove::ShiftTab, false); // do not stay inside
                    setFocus(Qt::OtherFocusReason);
                    ke->accept();
                    return true;
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}


void mdn::gui::NumberDisplayWidget::keyPressEvent(QKeyEvent* e) {
    if (m_editing) {
        return keyPressEvent_digitEditScope(e);
    } else {
        return keyPressEvent_gridScope(e);
    }
}


void mdn::gui::NumberDisplayWidget::keyPressEvent_gridScope(QKeyEvent* e) {

    if (!m_selection || !m_project) {
        Log_Warn("NumberDisplayWidget key press event handling with no valid project or selection");
        QWidget::keyPressEvent(e);
        return;
    }

    const Qt::KeyboardModifiers mods = e->modifiers();
    const bool shift  = mods.testFlag(Qt::ShiftModifier);
    const bool ctrl   = mods.testFlag(Qt::ControlModifier);
    const bool meta   = mods.testFlag(Qt::MetaModifier);
    const bool alt    = mods.testFlag(Qt::AltModifier);
    const bool extend = shift;

    if (ctrl || meta) {
        // Tab selection / movement shortcuts: Ctrl+PgUp, Ctrl+Shift+PgUp, etc..
        if (e->key() == Qt::Key_PageDown) {
            if (shift) {
                Q_EMIT requestMoveTabRight();
            } else {
                Q_EMIT requestSelectNextTab();
            }
            e->accept();
            return;
        } else if (e->key() == Qt::Key_PageUp) {
            if (shift) {
                Q_EMIT requestMoveTabLeft();
            } else {
                Q_EMIT requestSelectPrevTab();
            }
            e->accept();
            return;
        }

        // Zoom shortcuts: Ctrl+'+' / Ctrl+'-' / Ctrl+'0'
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
        if (e->key() == Qt::Key_A) {
            selectAllBounds();
            e->accept();
            return;
        }
    }

    if (!ctrl && !meta && !alt) {
        if (e->key() == Qt::Key_Plus || e->key() == Qt::Key_Equal) {
            emit requestToggleEditMode(EditMode::Add);
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_Minus) {
            emit requestToggleEditMode(EditMode::Subtract);
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_F2) {
            beginCellEdit(QString());
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_F3) {
            emit requestCycleEditMode(!shift);
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_F4) {
            emit requestCycleFraxis();
            e->accept();
            return;
        }
    }

    // Navigation keys:
    switch (e->key()) {
        // ---- Arrow keys ----
        case Qt::Key_Up:
            if (ctrl) m_selection->cursorJumpUp(extend);
            else      m_selection->cursorUp(extend);
            e->accept();
            postNavRefresh();
            return;

        case Qt::Key_Down:
            if (ctrl) m_selection->cursorJumpDn(extend);
            else      m_selection->cursorDn(extend);
            e->accept();
            postNavRefresh();
            return;

        case Qt::Key_Left:
            if (ctrl) m_selection->cursorJumpLf(extend);
            else      m_selection->cursorLf(extend);
            e->accept();
            postNavRefresh();
            return;

        case Qt::Key_Right:
            if (ctrl) m_selection->cursorJumpRt(extend);
            else      m_selection->cursorRt(extend);
            e->accept();
            postNavRefresh();
            return;

        // ---- Paging ----
        case Qt::Key_PageUp:
            if (alt)  m_selection->cursorPageLf(extend);
            else      m_selection->cursorPageUp(extend);
            e->accept();
            postNavRefresh();
            return;

        case Qt::Key_PageDown:
            if (alt)  m_selection->cursorPageRt(extend);
            else      m_selection->cursorPageDn(extend);
            e->accept();
            postNavRefresh();
            return;

        // ---- Origin ----
        case Qt::Key_Home: {
            if (ctrl) {
                m_selection->cursorOrigin(extend);
                m_cursorX = 0;
                m_cursorY = 0;
                centreViewOnOrigin();
            } else {
                m_cursorX = m_selection->cursor1().x();
                m_cursorY = m_selection->cursor1().y();
                centreViewOn(m_cursorX, m_cursorY);
            }
            e->accept();
            postNavRefresh();
            return;
        }

        // ---- Next / Prev by “entry” convention ----
        // Enter / Return: move along Y (down by default)
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (shift) {
                m_selection->cursorIterateY();
            } else {
                m_selection->cursorIterateReverseY();
            }
            e->accept();
            postNavRefresh();
            return;

        // Tab: move along X (right by default)
        case Qt::Key_Tab:
            m_selection->cursorIterateX();
            e->accept();
            postNavRefresh();
            return;

        // Shift+Tab comes in as Key_Backtab on some platforms
        case Qt::Key_Backtab:
            m_selection->cursorIterateReverseX();
            e->accept();
            postNavRefresh();
            return;

        case Qt::Key_Backspace:
        case Qt::Key_Delete: {
            if (!ctrl && !meta && !alt && m_selection && m_model) {
                m_model->setToZero(m_selection->rect());
                e->accept();
                postNavRefresh();
            }
        }


        // Debugging purposes
        case Qt::Key_Space: {
            if (!ctrl && !meta && !alt) {
                Q_EMIT requestDebugShowAllTabs();
                e->accept();
                return;
            }
            QWidget::keyPressEvent(e);
            return;
        }

        case Qt::Key_Escape:
            clearSelection();
            // emit focusDownRequested();
            e->accept();
            return;

        default:
            break;
    }

    // Ensure the caret stays visible and repaint immediately
    if (m_selection) {
        m_cursorX = m_selection->cursor1().x();
        m_cursorY = m_selection->cursor1().y();
        ensureCursorVisible();
        emit statusCursorChanged(m_cursorX, m_cursorY);
        emit statusSelectionChanged(*m_selection);
    }
    if (isGridTypingKey(e)) {
        const QString ch = e->text();
        beginCellEdit(ch);
        return;
    }

    if (!e->isAccepted()) {
        QWidget::keyPressEvent(e);
    }
    update();
}


void mdn::gui::NumberDisplayWidget::keyPressEvent_digitEditScope(QKeyEvent* e) {
    const Qt::KeyboardModifiers mods = e->modifiers();
    const bool shift  = mods.testFlag(Qt::ShiftModifier);
    const bool ctrl   = mods.testFlag(Qt::ControlModifier);
    const bool meta   = mods.testFlag(Qt::MetaModifier);
    const bool alt    = mods.testFlag(Qt::AltModifier);

    switch (e->key()) {
        // Submit
        case Qt::Key_Return:
        case Qt::Key_Enter: {
            SubmitMove how = shift ? SubmitMove::ShiftEnter : SubmitMove::Enter;
            commitCellEdit(how, true); // stay inside
            return;
        }
        case Qt::Key_Tab: {
            commitCellEdit(SubmitMove::Tab, true); // stay inside
            return;
        }
        case Qt::Key_Backtab: {
            commitCellEdit(SubmitMove::ShiftTab, true); // stay inside
            return;
        }
        case Qt::Key_Escape:
        case Qt::Key_F2: {
            cancelCellEdit();
            return;
        }
        case Qt::UpArrow: {
            commitCellEdit(SubmitMove::ShiftEnter, false); // do not stay inside
            return;
        }
        case Qt::DownArrow: {
            commitCellEdit(SubmitMove::Enter, false); // do not stay inside
            return;
        }
        case Qt::LeftArrow: {
            commitCellEdit(SubmitMove::ShiftTab, false); // do not stay inside
            return;
        }
        case Qt::RightArrow: {
            commitCellEdit(SubmitMove::Tab, false); // do not stay inside
            return;
        }
    }

    QWidget::keyPressEvent(e);
    return;
}


void mdn::gui::NumberDisplayWidget::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        int mx{0};
        int my{0};
        pixelToModel(e->pos().x(), e->pos().y(), mx, my);

        if (e->modifiers().testFlag(Qt::ShiftModifier)) {
            m_dragging = true;
            if (m_selection) {
                m_dragAnchorX = m_selection->cursor0().x();
                m_dragAnchorY = m_selection->cursor0().y();
            } else {
                m_dragAnchorX = m_cursorX;
                m_dragAnchorY = m_cursorY;
            }
            setCursor1(mx, my);
        } else {
            m_dragging = true;
            m_dragAnchorX = mx;
            m_dragAnchorY = my;
            setBothCursors(mx, my);
        }
        e->accept();
        return;
    }
    QWidget::mousePressEvent(e);
}


void mdn::gui::NumberDisplayWidget::mouseMoveEvent(QMouseEvent* e) {
    if (m_dragging) {
        int mx{0};
        int my{0};
        pixelToModel(e->pos().x(), e->pos().y(), mx, my);
        if (m_selection) {
            m_selection->setCursor0({m_dragAnchorX, m_dragAnchorY});
            m_selection->setCursor1({mx, my});
            m_selection->syncRectToCursors();
        }
        m_cursorX = mx;
        m_cursorY = my;
        ensureCursorVisible();
        update();
        e->accept();
        return;
    }
    QWidget::mouseMoveEvent(e);
}


void mdn::gui::NumberDisplayWidget::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_dragging = false;
        e->accept();
        return;
    }
    QWidget::mouseReleaseEvent(e);
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
    Log_Debug3_H("");
    captureCursorFractions();
    QWidget::resizeEvent(e);
    recalcGridGeometry();
    restoreCursorFractions();

    if (m_deferPostRestore) {
        m_deferPostRestore = false;
        restoreCursorFractions();
    }

    if (m_selection) {
        const int pageCols = std::max(1, (m_cols - 1) / 3);
        const int pageRows = std::max(1, (m_rows - 1) / 3);
        m_selection->setPageStep(pageCols, pageRows);
    }
    if (m_editing) {
        positionCellEditor();
    }
    if (m_armCentreViewOnOrigin) {
        centreViewOnOrigin();
        m_armCentreViewOnOrigin = false;
    }
    update();
    Log_Debug3_T("");
}


void mdn::gui::NumberDisplayWidget::changeEvent(QEvent* event)
{
    Log_Debug3_H("");
    if (event->type() == QEvent::WindowStateChange) {
        captureCursorFractions();
        m_deferPostRestore = true;
        update();
        Log_Debug3_T("");
        return;
    }

    QWidget::changeEvent(event);
    Log_Debug3_T("");
}


void mdn::gui::NumberDisplayWidget::recalcGridGeometry() {
    Log_Debug3_H("");
    QFontMetrics fm(m_theme.font);
    const int cw = fm.horizontalAdvance(QLatin1Char('0')) + 2;
    const int ch = fm.height() + 2;
    m_cellSize = std::max(cw, ch);

    m_cols = std::max(1, width() / m_cellSize) + 1;
    m_rows = std::max(1, height() / m_cellSize) + 1;

    const int pageCols = std::max(1, (m_cols - 1) / 3);
    const int pageRows = std::max(1, (m_rows - 1) / 3);
    m_project->setPageStep(pageCols, pageRows);
    Log_Debug3_T("");
}


void mdn::gui::NumberDisplayWidget::ensureCursorVisible() {
    Log_Debug3_H("");
    const int guardX = std::max(0, int(std::floor(double(m_cols) * kEdgeGuardFrac)));
    const int guardY = std::max(0, int(std::floor(double(m_rows) * kEdgeGuardFrac)));

    const int left = m_viewOriginX + guardX;
    const int right = m_viewOriginX + (m_cols - 1) - guardX;
    const int topY = m_viewOriginY + guardY;
    const int bottomY = m_viewOriginY + (m_rows - 1) - guardY;

    if (m_cursorX < left) {
        m_viewOriginX = m_cursorX - guardX;
    } else {
        if (m_cursorX > right) {
            m_viewOriginX = m_cursorX - ((m_cols - 1) - guardX);
        }
    }

    if (m_cursorY < topY) {
        m_viewOriginY = m_cursorY - guardY;
    } else {
        if (m_cursorY > bottomY) {
            m_viewOriginY = m_cursorY - ((m_rows - 1) - guardY);
        }
    }
    Log_Debug3_T("");
}


void mdn::gui::NumberDisplayWidget::drawAxes(QPainter& painter, const QRect& widgetRect) {
    // X axis in view?
    if (m_viewOriginY <= 0 && 0 < m_viewOriginY + m_rows) {
        const int yIndex = m_viewOriginY + m_rows;
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
    emit requestFontSizeChange(fontPointSize() + deltaPts);
}


void mdn::gui::NumberDisplayWidget::pixelToModel(int px, int py, int& mx, int& my) const {
    const int cs = std::max(1, m_cellSize);
    const int col = px / cs;
    const int row = py / cs;

    mx = m_viewOriginX + col;

    const int invRow = (m_rows > 0) ? ((m_rows - 1) - row) : 0;
    my = m_viewOriginY + invRow;
}


void mdn::gui::NumberDisplayWidget::centreViewOn(int mx, int my) {
    Log_Debug2_H("");
    const int halfCols = m_cols / 2;
    const int halfRows = m_rows / 2;

    m_viewOriginX = mx - halfCols;
    m_viewOriginY = my - halfRows;

    Log_Debug3(
        "viewOrigin=(" << m_viewOriginX << "," << m_viewOriginY << "), "
            << "viewSize=(" << m_cols << "," << m_rows << ")"
    );
    update();
    Log_Debug2_T("");
}


void mdn::gui::NumberDisplayWidget::centreViewOnOrigin() {
    Log_Debug2_H("");
    centreViewOn(0, 0);
    Log_Debug2_T("");
}


void mdn::gui::NumberDisplayWidget::armCentreViewOnOrigin() {
    m_armCentreViewOnOrigin = true;
}


void mdn::gui::NumberDisplayWidget::selectAllBounds()
{
    Log_Debug3_H("");

    if (!m_model || !m_selection) {
        Log_Debug3_T("");
        return;
    }

    const Rect b = m_model->bounds();
    if (b.isInvalid()) {
        Log_Debug3_T("Bounds empty");
        return;
    }

    m_selection->setRect(b);
    postNavRefresh();

    Log_Debug3_T("");
}


void mdn::gui::NumberDisplayWidget::captureCursorFractions() {
    if (m_cols <= 1) {
        m_lastCursorFracX = 0.5;
    } else {
        m_lastCursorFracX = double(m_cursorX - m_viewOriginX) / double(m_cols - 1);
    }
    if (m_rows <= 1) {
        m_lastCursorFracY = 0.5;
    } else {
        m_lastCursorFracY = double(m_cursorY - m_viewOriginY) / double(m_rows - 1);
    }
    if (m_lastCursorFracX < 0.0) {
        m_lastCursorFracX = 0.0;
    } else {
        if (m_lastCursorFracX > 1.0) {
            m_lastCursorFracX = 1.0;
        }
    }
    if (m_lastCursorFracY < 0.0) {
        m_lastCursorFracY = 0.0;
    } else {
        if (m_lastCursorFracY > 1.0) {
            m_lastCursorFracY = 1.0;
        }
    }
}


void mdn::gui::NumberDisplayWidget::restoreCursorFractions() {
    int targetCol = int(std::round(m_lastCursorFracX * double(std::max(1, m_cols - 1))));
    int targetRow = int(std::round(m_lastCursorFracY * double(std::max(1, m_rows - 1))));
    m_viewOriginX = m_cursorX - targetCol;
    m_viewOriginY = m_cursorY - targetRow;
    ensureCursorVisible();
}


void mdn::gui::NumberDisplayWidget::setBothCursors(int mx, int my) {
    m_cursorX = mx;
    m_cursorY = my;
    if (m_selection) {
        m_selection->setCursor0({mx, my});
        m_selection->setCursor1({mx, my});
        m_selection->syncRectToCursors();
    }
    ensureCursorVisible();
    emit statusCursorChanged(m_cursorX, m_cursorY);
    emit statusSelectionChanged(*m_selection);
    update();
}


void mdn::gui::NumberDisplayWidget::setCursor1(int mx, int my) {
    m_cursorX = mx;
    m_cursorY = my;
    if (m_selection) {
        m_selection->setCursor1({mx, my});
        m_selection->syncRectToCursors();
    }
    ensureCursorVisible();
    emit statusCursorChanged(m_cursorX, m_cursorY);
    emit statusSelectionChanged(*m_selection);
    update();
}


void mdn::gui::NumberDisplayWidget::clearSelection()
{
    Log_Debug3_H("");

    if (!m_model || !m_selection) {
        Log_Debug3_T("");
        return;
    }

    m_selection->clear();
    postNavRefresh();

    Log_Debug3_T("");
}


void mdn::gui::NumberDisplayWidget::postNavRefresh() {
    if (!m_selection) {
        return;
    }
    m_cursorX = m_selection->cursor1().x();
    m_cursorY = m_selection->cursor1().y();
    ensureCursorVisible();
    emit statusCursorChanged(m_cursorX, m_cursorY);
    emit statusSelectionChanged(*m_selection);
    update();
}


void mdn::gui::NumberDisplayWidget::beginCellEdit(const QString& initialText)
{
    Log_Debug3_H("");
    if (!m_cellEditor) {
        m_cellEditor = new CellLineEdit(this);
        m_cellEditor->setFocusPolicy(Qt::StrongFocus);
        m_cellEditor->setFrame(true);
        m_cellEditor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        m_cellEditor->setFont(m_theme.font);
        m_cellEditor->installEventFilter(this);
    }
    m_cellEditor->setFont(m_theme.font);
    m_cellEditor->setText(initialText);
    positionCellEditor();
    m_cellEditor->show();
    m_cellEditor->raise();
    m_cellEditor->setFocus(Qt::OtherFocusReason);
    m_editing = true;
    update();
    Log_Debug3_T("");
}


void mdn::gui::NumberDisplayWidget::positionCellEditor()
{
    Log_Debug4_H("");
    const QRect cell = cursorCellRectInWidget();
    int x = cell.x();
    int y = cell.y();
    int h = cell.height();
    int maxW = width() - x;
    int w = std::max(1, maxW);
    m_cellEditor->setGeometry(QRect(x, y, w, h));
    Log_Debug4_T("");
}


void mdn::gui::NumberDisplayWidget::commitCellEdit(SubmitMove how, bool stayInside)
{
    Log_Debug2_H("");
    if (!m_model || !m_selection || !m_editing) {
        setFocus(Qt::OtherFocusReason);
        Log_Debug2_T("");
        return;
    }

    const QString raw = m_cellEditor->text().trimmed();
    const Mdn2dConfig& config = m_model->config();
    const int base = config.base();
    Log_Debug3("trimmed=" << raw.toStdString() << ",base=" << base);
    bool negative;
    std::string intPart;
    std::string fracPart;
    int radix;
    std::string message = Tools::verifyStringAsReal(
        raw.toStdString(), base,
        negative, intPart, fracPart, radix
    );
    Log_Debug3("trimmed=" << raw.toStdString() << ",base=" << base);

    if (message.size()) {
        // Not a valid string input - already logged
        emit requestStatus(QString::fromStdString(message), 2000);
        m_cellEditor->hide();
        m_editing = false;
        update();
        moveCursorAfterSubmit(how, stayInside);
        setFocus(Qt::OtherFocusReason);
        Log_Debug2_T("rejected text");
        return;
    }

    if (m_mode == EditMode::Subtract) {
        negative = !negative;
    }
    bool overwrite = m_mode == EditMode::Overwrite;
    const mdn::Coord xy = m_selection->cursor1();
    const Fraxis fraxis(config.fraxis());
    Log_Debug3("dispatch model->add(xy=" << xy << ",neg=" << negative << ",...)");
    m_model->add(
        xy,
        negative,
        intPart,
        fracPart,
        overwrite,
        m_model->config().fraxis()
    );
    Log_Debug3(""
        << "return model->add(xy=" << xy << ",neg=" << negative
        << ",intPart=" << intPart << ",fracPart=" << fracPart
        << ",overwrite=" << overwrite << ",fraxis=" << FraxisToName(fraxis)
        << ")"
    );

    m_cellEditor->hide();
    m_editing = false;
    update();
    moveCursorAfterSubmit(how, stayInside);
    setFocus(Qt::OtherFocusReason);
    Log_Debug2_T("");
}


bool mdn::gui::NumberDisplayWidget::isGridTypingKey(const QKeyEvent* ev) const
{
    const Qt::KeyboardModifiers mods = ev->modifiers();
    if ((mods & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) != Qt::NoModifier) {
        return false;
    }

    if (!m_model) {
        return false;
    }
    int base = m_model->config().base();

    // Set key limits based on base
    // * maxNum is at-the-end, i.e. k <= maxNum, but maxAlpha is different
    // * maxAlpha is one-past-the-end, i.e. k < maxAlpha, not k <= maxAlpha
    Qt::Key maxNum = Qt::Key_9;
    Qt::Key maxAlpha = Qt::Key_A;
    switch (base) {
        case 2: maxNum = Qt::Key_1; break;
        case 3: maxNum = Qt::Key_2; break;
        case 4: maxNum = Qt::Key_3; break;
        case 5: maxNum = Qt::Key_4; break;
        case 6: maxNum = Qt::Key_5; break;
        case 7: maxNum = Qt::Key_6; break;
        case 8: maxNum = Qt::Key_7; break;
        case 9: maxNum = Qt::Key_8; break;
        case 10: break;
        case 11: maxAlpha = Qt::Key_A; break;
        case 12: maxAlpha = Qt::Key_B; break;
        case 13: maxAlpha = Qt::Key_C; break;
        case 14: maxAlpha = Qt::Key_D; break;
        case 15: maxAlpha = Qt::Key_E; break;
        case 16: maxAlpha = Qt::Key_F; break;
        case 17: maxAlpha = Qt::Key_G; break;
        case 18: maxAlpha = Qt::Key_H; break;
        case 19: maxAlpha = Qt::Key_I; break;
        case 20: maxAlpha = Qt::Key_J; break;
        case 21: maxAlpha = Qt::Key_K; break;
        case 22: maxAlpha = Qt::Key_L; break;
        case 23: maxAlpha = Qt::Key_M; break;
        case 24: maxAlpha = Qt::Key_N; break;
        case 25: maxAlpha = Qt::Key_O; break;
        case 26: maxAlpha = Qt::Key_P; break;
        case 27: maxAlpha = Qt::Key_Q; break;
        case 28: maxAlpha = Qt::Key_R; break;
        case 29: maxAlpha = Qt::Key_S; break;
        case 30: maxAlpha = Qt::Key_T; break;
        case 31: maxAlpha = Qt::Key_U; break;
        case 32: maxAlpha = Qt::Key_V; break;
        default:
            Log_Warn("Invalid base: " << base);
    }
    const int k = ev->key();
    if (k == Qt::Key_Minus || k == Qt::Key_Period) {
        return true;
    }
    if ((k >= Qt::Key_0 && k <= maxNum) || (k >= Qt::Key_A && k < maxAlpha)) {
        return true;
    }
    return false;
}


bool mdn::gui::NumberDisplayWidget::textAcceptableForBase(const QString& s, int base) const
{
    if (s.isEmpty()) {
        return false;
    }

    QString t = s.trimmed();
    if (t.startsWith('+')) {
        t.remove(0, 1);
    }

    bool neg = false;
    if (t.startsWith('-')) {
        neg = true;
        t.remove(0, 1);
    }

    if (t.isEmpty()) {
        return false;
    }

    int dots = 0;
    for (int i = 0; i < t.size(); ++i) {
        const QChar ch = t.at(i);
        if (ch == '.') {
            dots += 1;
            if (dots > 1) {
                return false;
            }
        } else {
            const int dv = charToDigitValue(ch);
            if (dv < 0) {
                return false;
            }
            if (dv >= base) {
                return false;
            }
        }
    }

    return true;
}


int mdn::gui::NumberDisplayWidget::charToDigitValue(QChar ch) const
{
    const QChar c = GuiTools::toUpperAscii(ch);
    if (c >= QChar('0') && c <= QChar('9')) {
        return c.unicode() - QChar('0').unicode();
    }
    if (c >= QChar('A') && c <= QChar('Z')) {
        return 10 + (c.unicode() - QChar('A').unicode());
    }
    return -1;
}


double mdn::gui::NumberDisplayWidget::parseBaseReal(const QString& s, int base, bool& ok) const
{
    ok = false;

    if (!textAcceptableForBase(s, base)) {
        return 0.0;
    }

    QString t = s.trimmed();
    bool neg = false;

    if (t.startsWith('+')) {
        t.remove(0, 1);
    }

    if (t.startsWith('-')) {
        neg = true;
        t.remove(0, 1);
    }

    int dot = t.indexOf('.');
    QString intPart = dot < 0 ? t : t.left(dot);
    QString fracPart = dot < 0 ? QString() : t.mid(dot + 1);

    double value = 0.0;
    for (int i = 0; i < intPart.size(); ++i) {
        const int dv = charToDigitValue(intPart.at(i));
        value = value * base + dv;
    }

    double scale = 1.0;
    for (int i = 0; i < fracPart.size(); ++i) {
        const int dv = charToDigitValue(fracPart.at(i));
        scale /= static_cast<double>(base);
        value += static_cast<double>(dv) * scale;
    }

    if (neg) {
        value = -value;
    }

    ok = true;
    return value;
}


QRect mdn::gui::NumberDisplayWidget::cursorCellRectInWidget() const
{
    const int vx = m_cursorX - m_viewOriginX;
    const int vy = (m_rows - 1) - (m_cursorY - m_viewOriginY);
    const int px = vx * m_cellSize;
    const int py = vy * m_cellSize;
    return QRect(px, py, m_cellSize, m_cellSize);
}


void mdn::gui::NumberDisplayWidget::moveCursorAfterSubmit(SubmitMove how, bool stayInside)
{
    Log_Debug3_H("");
    if (!m_selection) {
        Log_Debug3_T("");
        return;
    }

    const Rect r = m_selection->rect();
    const Coord cur = m_selection->cursor1();
    if (stayInside) {
        switch (how) {
            case SubmitMove::Enter:
                m_selection->cursorIterateReverseY();
                break;
            case SubmitMove::ShiftEnter:
                m_selection->cursorIterateY();
                break;
            case SubmitMove::Tab:
                m_selection->cursorIterateX();
                break;
            case SubmitMove::ShiftTab:
                m_selection->cursorIterateReverseX();
                break;
        }
    } else {
        switch (how) {
            case SubmitMove::Enter:
                m_selection->cursorPrevY(false);
                break;
            case SubmitMove::ShiftEnter:
                m_selection->cursorNextY(false);
                break;
            case SubmitMove::Tab:
                m_selection->cursorNextX(false);
                break;
            case SubmitMove::ShiftTab:
                m_selection->cursorPrevX(false);
                break;
        }
    }
    postNavRefresh();
    Log_Debug3_T("");
    return;
}


QString mdn::gui::NumberDisplayWidget::stripSign(const QString& s, bool& isNeg) const
{
    QString t(s.trimmed());
    isNeg = false;
    if (t.startsWith('+')) {
        t.remove(0, 1);
    } else if (t.startsWith('-')) {
        isNeg = true;
        t.remove(0, 1);
    }
    return t;
}


double mdn::gui::NumberDisplayWidget::parseBaseRealMagnitude(
    const QString& body,
    int base,
    int& nFracDigitsOut,
    bool& ok
) const
{
    ok = false;
    QString intPart;
    QString fracPart;
    const int dot = body.indexOf('.');
    if (dot < 0) {
        intPart = body;
    } else {
        intPart = body.left(dot);
        fracPart = body.mid(dot + 1);
    }
    nFracDigitsOut = fracPart.size();

    double value = 0.0;
    for (int i = 0; i < intPart.size(); ++i) {
        const int dv = charToDigitValue(intPart.at(i));
        value = value * base + dv;
    }

    double scale = 1.0;
    for (int i = 0; i < fracPart.size(); ++i) {
        const int dv = charToDigitValue(fracPart.at(i));
        scale /= static_cast<double>(base);
        value += static_cast<double>(dv) * scale;
    }

    ok = true;
    return value;
}


long long mdn::gui::NumberDisplayWidget::parseBaseIntMagnitude(const QString& body, int base, bool& ok) const
{
    ok = false;
    long long value = 0;
    for (int i = 0; i < body.size(); ++i) {
        const int dv = charToDigitValue(body.at(i));
        value = value * base + dv;
    }
    ok = true;
    return value;
}


QString mdn::gui::NumberDisplayWidget::modeShortText() const {
    switch (m_mode) {
        case EditMode::Overwrite: {
            return QStringLiteral("OVER");
        }
        case EditMode::Add: {
            return QStringLiteral("ADD +");
        }
        case EditMode::Subtract: {
            return QStringLiteral("SUB −");
        }
    }
    return QStringLiteral("OVER");
}


QString mdn::gui::NumberDisplayWidget::selectionSummaryText() const {
    if (!m_selection) return QString();
    const Rect r = m_selection->rect();
    if (!r.isValid()) return QStringLiteral("(empty)");
    const int w = r.width();
    const int h = r.height();
    return QStringLiteral("[%1..%2]×[%3..%4]  (%5×%6)")
        .arg(r.left()).arg(r.right())
        .arg(r.bottom()).arg(r.top())
        .arg(w).arg(h);
}
