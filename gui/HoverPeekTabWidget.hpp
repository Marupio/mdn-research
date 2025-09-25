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

// Forward declarations
class MarkerWidget;
class NumberDisplayWidget;

class HoverPeekTabWidget : public QTabWidget {
    Q_OBJECT
public:
    explicit HoverPeekTabWidget(QWidget* parent = nullptr);

    void setPlusTab(int idx, MarkerWidget* page);
    bool hasPlus() const { return m_plusMarker && indexOf(m_plusMarkerBase) >= 0; }
    int plusIndex() const { return m_plusMarker ? indexOf(m_plusMarkerBase) : -1; }

    // Returns the currentIndex that is actually selected, not hover-peeked
    int currentSelectedIndex() const { return m_lastPermanentIndex; }

    // Optional settings
    void setHoverDelayMs(int ms) { m_hoverDelayMs = std::max(0, ms); }
    void setPreviewTextColor(const QColor& c) { m_previewTextColor = c; }
    void clearPreviewTextColor() { m_previewTextColor = QColor(); }

    // Sizing
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;


signals:
    void beganPreview(int hoveredIndex);
    void endedPreview(int restoredIndex);
    void committedIndex(int index);
    void plusClicked();

private slots:
    void onTabMoved(int from, int to);
    void onTabBarClicked(int index);
    void onCurrentChangedGuard(int index);
    void onHoverIndex(int idx);
    void onHoverEnd();
    void onCommitIndex(int idx);

protected:
    void tabInserted(int index) override {
        if (m_lastPermanentIndex < 0) {
            m_lastPermanentIndex = 0;
        }
    }

private:
    void enforcePlusAtEnd();
    // page-safe move
    void moveIndex(int from, int to);
    void restoreIfPreviewing();
    void beginPreviewHighlight(int idx);
    void endPreviewHighlight(int idx);
    NumberDisplayWidget* ndwAt(int idx) const;

private:
    QTimer m_restoreTimer;
    int    m_hoverDelayMs { 120 };  // adjust to taste
    int    m_plusTabIndex { -1 };   // index of "+" tab (ignored for previews)
    int    m_lastPermanentIndex { -1 };
    int    m_pendingHoverIndex { -1 };
    bool   m_previewActive { false };
    QColor m_previewTextColor { Qt::blue }; // subtle cue; change or clear per taste

    MarkerWidget* m_plusMarker = nullptr;
    QWidget* m_plusMarkerBase = nullptr;
    int m_lastRealIndex = -1;
};

} // end namespace gui
} // end namespace mdn
