#include "HoverPeekTabWidget.hpp"
#include <QTabBar>
#include <QSignalBlocker>

#include "HoverPeekTabBar.hpp"
#include "MarkerWidget.hpp"

mdn::gui::HoverPeekTabWidget::HoverPeekTabWidget(QWidget* parent) :
    QTabWidget(parent)
{
    auto* bar = new HoverPeekTabBar(this);
    QTabWidget::setTabBar(bar);
    setMovable(true);

    connect(bar, &HoverPeekTabBar::hoverIndex, this, &HoverPeekTabWidget::onHoverIndex);
    connect(bar, &HoverPeekTabBar::hoverEnd, this, &HoverPeekTabWidget::onHoverEnd);
    connect(bar, &HoverPeekTabBar::commitIndex, this, &HoverPeekTabWidget::onCommitIndex);

    m_restoreTimer.setSingleShot(true);
    // we call start() with the chosen delay
    m_restoreTimer.setInterval(120);
    connect(&m_restoreTimer, &QTimer::timeout, this, &HoverPeekTabWidget::restoreIfPreviewing);
    connect(tabBar(), &QTabBar::tabMoved, this, &mdn::gui::HoverPeekTabWidget::onTabMoved);
    connect(tabBar(), &QTabBar::tabBarClicked, this, &HoverPeekTabWidget::onTabBarClicked);
    connect(this, &QTabWidget::currentChanged, this, &HoverPeekTabWidget::onCurrentChangedGuard);
}


void mdn::gui::HoverPeekTabWidget::setPlusTab(int idx, MarkerWidget* page) {
    m_plusTabIndex = idx;
    if (page) {
        m_plusMarker = page;
        m_plusMarkerBase = static_cast<QWidget*>(page);
    } else {
        m_plusMarker = nullptr;
        m_plusMarkerBase = nullptr;
    }
    enforcePlusAtEnd();
}


void mdn::gui::HoverPeekTabWidget::onTabMoved(int from, int to) {
    // If a real tab was dragged “past” the end, it’ll land at 'last'.
    // We want [+] to remain last, so nudge that tab back before plus.
    const int last = count() - 1;
    if (to == last && m_plusMarker && widget(last) == m_plusMarker) {
        // Nothing to do: plus is still last.
    } else if (m_plusMarker && indexOf(m_plusMarker) == last && to == last) {
        // Real tab landed after plus (illegal invariant). Put it before plus.
        moveIndex(last, last);        // normalize indexes post-move (no-op safety)
        moveIndex(count()-1, count()-1); // no-op; placeholder in case of style quirks
        moveIndex(count()-1, count()-2); // move the *real* tab back before last
    }

    // Final guard: always ensure [+] is last.
    enforcePlusAtEnd();
}


void mdn::gui::HoverPeekTabWidget::onTabBarClicked(int index)
{
    // If [+] was clicked, emit a signal to create a new tab and keep focus on a real tab.
    if (index >= 0 && index == plusIndex()) {
        emit plusClicked();
        // Keep selection on last real tab (before +) if any
        const int lastReal = count() - (hasPlus() ? 2 : 1);
        if (lastReal >= 0) setCurrentIndex(lastReal);
    } else {
        m_lastRealIndex = index;
    }
}


void mdn::gui::HoverPeekTabWidget::onCurrentChangedGuard(int index)
{
    // If something programmatically set current to the plus page, bounce off it.
    if (index == plusIndex()) {
        emit plusClicked();
        if (m_lastRealIndex >= 0 && m_lastRealIndex < count() && m_lastRealIndex != index)
            setCurrentIndex(m_lastRealIndex);
        else if (int lastReal = count() - (hasPlus() ? 2 : 1); lastReal >= 0)
            setCurrentIndex(lastReal);
    } else {
        m_lastRealIndex = index;
    }
}


void mdn::gui::HoverPeekTabWidget::onHoverIndex(int idx) {
    if (idx < 0 || idx >= count()) return;
    if (idx == m_plusTabIndex) return; // don't preview the "+" tab

    // Debounce/flicker control: delay switching until the mouse settles a bit
    m_pendingHoverIndex = idx;
    m_restoreTimer.stop();
    m_restoreTimer.start(m_hoverDelayMs);
}


void mdn::gui::HoverPeekTabWidget::onHoverEnd() {
    // Pointer left the bar or isn't on any tab anymore: cancel pending
    m_restoreTimer.stop();
    m_pendingHoverIndex = -1;

    // If we were previewing, restore immediately for a snappy feel
    if (m_previewActive) {
        restoreIfPreviewing();
    }
}


void mdn::gui::HoverPeekTabWidget::onCommitIndex(int idx) {
    if (idx < 0 || idx >= count()) return;
    if (idx == m_plusTabIndex) return; // "+" tab commit is likely handled by app-level code

    // Make this the permanent selection and end preview if any
    endPreviewHighlight(currentIndex());
    m_lastPermanentIndex = idx;
    if (m_previewActive) {
        m_previewActive = false;
        emit endedPreview(m_lastPermanentIndex);
    }
    setCurrentIndex(idx);
    emit committedIndex(idx);
}


void mdn::gui::HoverPeekTabWidget::enforcePlusAtEnd()
{
    if (!m_plusMarker) {
        return;
    }

    const int last = count() - 1;
    const int plus = indexOf(m_plusMarker);
    if (plus < 0) {
        return;
    }

    if (plus != last) {
        // Move the PLUS PAGE itself to the very end
        moveIndex(plus, count()); // insert at end
        if (currentWidget() == m_plusMarker && last >= 0)
            setCurrentIndex(last); // focus previous last (now last-1)
    }
}


void mdn::gui::HoverPeekTabWidget::moveIndex(int from, int to) {
    // if (from == to || from < 0 || to < 0 || from >= count()) return;
    if (from == to || from < 0 || from >= count()) return;
    if (to < 0) to = 0;
    if (to > count()) to = count();

    QSignalBlocker b1(this);
    QSignalBlocker b2(tabBar());

    QWidget* w = widget(from);
    const QIcon ic = tabIcon(from);
    const QString tx = tabText(from);
    const QVariant data = tabBar()->tabData(from);
    const QString tip = tabToolTip(from);
    const QString wt = tabWhatsThis(from);

    removeTab(from);
    // account for removal shift
    if (to > from) --to;
    const int newIdx = insertTab(to, w, ic, tx);
    setTabToolTip(newIdx, tip);
    setTabWhatsThis(newIdx, wt);
    tabBar()->setTabData(newIdx, data);
}


void mdn::gui::HoverPeekTabWidget::restoreIfPreviewing() {
    // If there is a pending hover target, switch to it now (this is the delayed "preview")
    if (m_pendingHoverIndex >= 0 && m_pendingHoverIndex < count()) {
        if (!m_previewActive) {
            m_lastPermanentIndex = currentIndex();
            m_previewActive = true;
            emit beganPreview(m_pendingHoverIndex);
        }
        // Change preview highlight
        endPreviewHighlight(currentIndex());
        setCurrentIndex(m_pendingHoverIndex);
        beginPreviewHighlight(m_pendingHoverIndex);
        m_pendingHoverIndex = -1;
        return;
    }

    // Otherwise, restore original tab if we are in preview mode
    if (m_previewActive) {
        const int curr = currentIndex();
        endPreviewHighlight(curr);
        if (m_lastPermanentIndex >= 0 && m_lastPermanentIndex < count()
            && m_lastPermanentIndex != curr) {
            setCurrentIndex(m_lastPermanentIndex);
        }
        m_previewActive = false;
        emit endedPreview(currentIndex());
    }
}


void mdn::gui::HoverPeekTabWidget::beginPreviewHighlight(int idx) {
    if (auto* b = tabBar()) {
        if (m_previewTextColor.isValid())
            b->setTabTextColor(idx, m_previewTextColor);
    }
}


void mdn::gui::HoverPeekTabWidget::endPreviewHighlight(int idx) {
    if (auto* b = tabBar()) {
        b->setTabTextColor(idx, QColor()); // default palette
    }
}
