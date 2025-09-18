#pragma once
#include <QTabWidget>
#include <QTimer>
#include <QPointer>

#include "HoverPeekTabBar.hpp"

// A QTabWidget that preview-switches pages while hovering tabs.
// The switch is temporary until the user clicks a tab ("commit").
// You can mark a "plus" tab (by index) to be ignored by previews.

namespace mdn {
namespace gui {

class HoverPeekTabWidget : public QTabWidget {
    Q_OBJECT
public:
    explicit HoverPeekTabWidget(QWidget* parent = nullptr)
        : QTabWidget(parent)
    {
        // IMPORTANT: setTabBar is protected; we can call it here because we're a subclass.
        auto* bar = new HoverPeekTabBar(this);
        QTabWidget::setTabBar(bar);

        connect(bar, &HoverPeekTabBar::hoverIndex, this, &HoverPeekTabWidget::onHoverIndex);
        connect(bar, &HoverPeekTabBar::hoverEnd, this, &HoverPeekTabWidget::onHoverEnd);
        connect(bar, &HoverPeekTabBar::commitIndex, this, &HoverPeekTabWidget::onCommitIndex);

        m_restoreTimer.setSingleShot(true);
        m_restoreTimer.setInterval(120); // we call start() with the chosen delay
        connect(&m_restoreTimer, &QTimer::timeout, this, &HoverPeekTabWidget::restoreIfPreviewing);
    }

    // Optional settings
    void setHoverDelayMs(int ms) { m_hoverDelayMs = std::max(0, ms); }
    void setPlusTabIndex(int idx) { m_plusTabIndex = idx; } // -1 means none
    void setPreviewTextColor(const QColor& c) { m_previewTextColor = c; }
    void clearPreviewTextColor() { m_previewTextColor = QColor(); }

signals:
    void beganPreview(int hoveredIndex);
    void endedPreview(int restoredIndex);
    void committedIndex(int index);

private slots:
    void onHoverIndex(int idx) {
        if (idx < 0 || idx >= count()) return;
        if (idx == m_plusTabIndex) return; // don't preview the "+" tab

        // Debounce/flicker control: delay switching until the mouse settles a bit
        m_pendingHoverIndex = idx;
        m_restoreTimer.stop();
        m_restoreTimer.start(m_hoverDelayMs);
    }

    void onHoverEnd() {
        // Pointer left the bar or isn't on any tab anymore: cancel pending
        m_restoreTimer.stop();
        m_pendingHoverIndex = -1;

        // If we were previewing, restore immediately for a snappy feel
        if (m_previewActive) {
            restoreIfPreviewing();
        }
    }

    void onCommitIndex(int idx) {
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

private:
    void restoreIfPreviewing() {
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

    void beginPreviewHighlight(int idx) {
        if (auto* b = tabBar()) {
            if (m_previewTextColor.isValid())
                b->setTabTextColor(idx, m_previewTextColor);
        }
    }

    void endPreviewHighlight(int idx) {
        if (auto* b = tabBar()) {
            b->setTabTextColor(idx, QColor()); // default palette
        }
    }

private:
    QTimer m_restoreTimer;
    int    m_hoverDelayMs { 120 };  // adjust to taste
    int    m_plusTabIndex { -1 };   // index of "+" tab (ignored for previews)
    int    m_lastPermanentIndex { -1 };
    int    m_pendingHoverIndex { -1 };
    bool   m_previewActive { false };
    QColor m_previewTextColor { Qt::blue }; // subtle cue; change or clear per taste
};

} // end namespace gui
} // end namespace mdn
