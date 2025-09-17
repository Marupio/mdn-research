#pragma once
#include <QTabBar>
#include <QHoverEvent>

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
    void peekIndex(int index);     // mouse is hovering over this tab
    void peekEnd();                // mouse left the tabbar (stop peeking)
    void commitIndex(int index);   // user clicked a tab (make it explicit)

protected:
    bool event(QEvent* e) override {
        if (e->type() == QEvent::HoverMove) {
            auto* he = static_cast<QHoverEvent*>(e);
            const int idx = tabAt(he->pos());
            if (idx >= 0) emit peekIndex(idx);
        }
        return QTabBar::event(e);
    }

    void leaveEvent(QEvent* e) override {
        emit peekEnd();
        QTabBar::leaveEvent(e);
    }

    void mousePressEvent(QMouseEvent* e) override {
        const int idx = tabAt(e->pos());
        if (idx >= 0) emit commitIndex(idx);
        QTabBar::mousePressEvent(e);
    }
};
