#include "HoverPeekTabBar.hpp"

#include <mdn/Logger.hpp>

// cross-version point helpers
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  #define EV_POINT(e) ((e)->position().toPoint())
  #define MS_POINT(e) ((e)->position().toPoint())
#else
  #define EV_POINT(e) ((e)->pos())
  #define MS_POINT(e) ((e)->pos())
#endif


mdn::gui::HoverPeekTabBar::HoverPeekTabBar(QWidget* parent) :
    QTabBar(parent)
{
    Log_Debug2_H("");
    setAcceptDrops(true);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    Log_Debug2_T("");
}


bool mdn::gui::HoverPeekTabBar::event(QEvent* e) {
    Log_Debug2_H("");
    if (e->type() == QEvent::HoverMove) {
        auto* he = static_cast<QHoverEvent*>(e);
        const int idx = tabAt(he->position().toPoint());
        if (idx >= 0) {
            Log_Debug3("emit hoverIndex(idx=" << idx << ")");
            emit hoverIndex(idx);
        } else {
            // <- cancel preview immediately when not over a tab
            Log_Debug3("emit hoverEnd()");
            emit hoverEnd();
        }
    }
    Log_Debug2_T("");
    return QTabBar::event(e);
}


void mdn::gui::HoverPeekTabBar::dragEnterEvent(QDragEnterEvent* e) {
    Log_Debug2_H("");
    const int i = tabAt(EV_POINT(e));
    if (i < 0 || isPlusIndex(i)) { e->ignore(); return; }
    e->acceptProposedAction();
    Log_Debug2_T("");
}


void mdn::gui::HoverPeekTabBar::dragMoveEvent(QDragMoveEvent* e) {
    Log_Debug2_H("");
    const int i = tabAt(EV_POINT(e));
    if (i < 0 || isPlusIndex(i)) {
        e->ignore();
        Log_Debug2_T("");
        return;
    }
    QTabBar::dragMoveEvent(e);
    Log_Debug2_T("");
}


void mdn::gui::HoverPeekTabBar::dropEvent(QDropEvent* e) {
    Log_Debug2_H("");
    const int i = tabAt(EV_POINT(e));
    if (i < 0 || isPlusIndex(i)) {
        e->ignore();
        Log_Debug2_T("");
        return;
    }
    QTabBar::dropEvent(e);
    Log_Debug2_T("");
}


void mdn::gui::HoverPeekTabBar::mousePressEvent(QMouseEvent* e) {
    Log_Debug2_H("");
    const int idx = tabAt(MS_POINT(e));
    if (idx >= 0) {
        if (!isPlusIndex(idx)) {
            Log_Debug3("emit commitIndex(idx=" << idx << ")");
            emit commitIndex(idx);
        }
    }
    QTabBar::mousePressEvent(e);
    Log_Debug2_T("");
}


void mdn::gui::HoverPeekTabBar::mouseMoveEvent(QMouseEvent* e) {
    Log_Debug2_H("");
    if (m_pressedIndex == count() - 1) {
        // Block drag initiation when [+] is the pressed tab.
        Log_Debug2_T("");
        return;
    }
    QTabBar::mouseMoveEvent(e);
    Log_Debug2_T("");
}


void mdn::gui::HoverPeekTabBar::mouseReleaseEvent(QMouseEvent* e) {
    Log_Debug2_H("");
    m_pressedIndex = -1;
    QTabBar::mouseReleaseEvent(e);
    Log_Debug2_T("");
}


void mdn::gui::HoverPeekTabBar::leaveEvent(QEvent* e) {
    Log_Debug2_H("");
    Log_Debug3("emit hoverEnd()");
    emit hoverEnd();
    QTabBar::leaveEvent(e);
    Log_Debug2_T("");
}
