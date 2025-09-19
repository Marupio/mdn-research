#include "HoverPeekTabBar.hpp"

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
    setAcceptDrops(true);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
}


bool mdn::gui::HoverPeekTabBar::event(QEvent* e) {
    if (e->type() == QEvent::HoverMove) {
        auto* he = static_cast<QHoverEvent*>(e);
        const int idx = tabAt(he->pos());
        if (idx >= 0) {
            emit hoverIndex(idx);
        } else {
            // <- cancel preview immediately when not over a tab
            emit hoverEnd();
        }
    }
    return QTabBar::event(e);
}


void mdn::gui::HoverPeekTabBar::dragEnterEvent(QDragEnterEvent* e) {
    const int i = tabAt(EV_POINT(e));
    if (i < 0 || isPlusIndex(i)) { e->ignore(); return; }
    e->acceptProposedAction();
}


void mdn::gui::HoverPeekTabBar::dragMoveEvent(QDragMoveEvent* e) {
    const int i = tabAt(EV_POINT(e));
    if (i < 0 || isPlusIndex(i)) { e->ignore(); return; }
    QTabBar::dragMoveEvent(e);
}


void mdn::gui::HoverPeekTabBar::dropEvent(QDropEvent* e) {
    const int i = tabAt(EV_POINT(e));
    if (i < 0 || isPlusIndex(i)) {
        e->ignore();
        return;
    }
    QTabBar::dropEvent(e);
}


void mdn::gui::HoverPeekTabBar::mousePressEvent(QMouseEvent* e) {
    const int idx = tabAt(MS_POINT(e));
    if (idx >= 0) emit commitIndex(idx);
    QTabBar::mousePressEvent(e);
}


void mdn::gui::HoverPeekTabBar::mouseMoveEvent(QMouseEvent* e) {
    if (m_pressedIndex == count() - 1) {
        // Block drag initiation when [+] is the pressed tab.
        return;
    }
    QTabBar::mouseMoveEvent(e);
}


void mdn::gui::HoverPeekTabBar::mouseReleaseEvent(QMouseEvent* e) {
    m_pressedIndex = -1;
    QTabBar::mouseReleaseEvent(e);
}


void mdn::gui::HoverPeekTabBar::leaveEvent(QEvent* e) {
    emit hoverEnd();
    QTabBar::leaveEvent(e);
}
