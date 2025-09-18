#pragma once
#include <QTabBar>
#include <QHoverEvent>
#include <QMouseEvent>

class HoverPeekTabBar : public QTabBar {
    Q_OBJECT
public:
    explicit HoverPeekTabBar(QWidget* parent = nullptr)
        : QTabBar(parent)
    {
        setMouseTracking(true);
        setAttribute(Qt::WA_Hover, true);
    }

signals:
    void hoverIndex(int index);   // mouse hovering a tab
    void hoverEnd();              // mouse left the tabbar
    void commitIndex(int index);  // user clicked a tab (explicit selection)

protected:
    bool event(QEvent* e) override {
        if (e->type() == QEvent::HoverMove) {
            auto* he = static_cast<QHoverEvent*>(e);
            const int idx = tabAt(he->pos());
            if (idx >= 0) emit hoverIndex(idx);
        }
        return QTabBar::event(e);
    }

    void leaveEvent(QEvent* e) override {
        emit hoverEnd();
        QTabBar::leaveEvent(e);
    }

    void mousePressEvent(QMouseEvent* e) override {
        const int idx = tabAt(e->pos());
        if (idx >= 0) emit commitIndex(idx);
        QTabBar::mousePressEvent(e);
    }
};
