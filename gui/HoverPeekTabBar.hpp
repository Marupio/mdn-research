#pragma once
#include <QTabBar>
#include <QHoverEvent>
#include <QMouseEvent>

namespace mdn {
namespace gui {

class HoverPeekTabBar : public QTabBar {
    Q_OBJECT
public:
    explicit HoverPeekTabBar(QWidget* parent = nullptr);

signals:
    // mouse hovering a tab
    void hoverIndex(int index);
    // mouse left the tabbar
    void hoverEnd();
    // user clicked a tab (explicit selection)
    void commitIndex(int index);

protected:
    bool event(QEvent* e) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dropEvent(QDropEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void leaveEvent(QEvent* e) override;

    bool isPlusIndex(int i) const { return i == count() - 1; } // last tab is [+]
    int m_pressedIndex = -1;
};

} // end namespace gui
} // end namespace mdn
