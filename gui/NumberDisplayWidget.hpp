#pragma once

#include <QWidget>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QPainter>

#include "../library/GlobalConfig.hpp"
#include "../library/Mdn2d.hpp"
#include "../library/Coord.hpp"
#include "../library/Rect.hpp"
#include "../library/Digit.hpp"
#include "Selection.hpp"

// Forward declarations
namespace mdn {
class Mdn2d;
class Selection;
}

namespace mdn {
namespace gui {

class NumberDisplayWidget : public QWidget {
    Q_OBJECT

public:
    struct Theme {
        QFont   font            {QStringLiteral("Courier New"), 11};
        QBrush  background      {Qt::NoBrush};
        QPen    gridPen         {QColor(160,160,160), 1};
        QPen    axisPen         {QColor(128,128,128), 3};
        QPen    originPen       {QColor(220,0,0), 2};
        QPen    textPen         {QColor(20,20,20)};
        QBrush  selectionFill   {QColor(82,143,255,70)};
        QBrush  anchorFill      {QColor(128,128,128,90)};
        QBrush  cursorFill      {QColor(255,214,64,140)};
    };


    explicit NumberDisplayWidget(QWidget* parent = nullptr);

    // Bindings
    void setProject(Project* proj);
    void setModel(const Mdn2d* mdn, Selection* sel);

    // Styling
    inline void setTheme(const Theme& t) {
        m_theme = t;
        recalcGridGeometry();
        update();
    }
    inline const Theme& theme() const {
        return m_theme;
    }

    // Zoom controls (font size)
    void increaseFont();   // Ctrl + '+'
    void decreaseFont();   // Ctrl + '-'
    void resetFont();      // Ctrl + '0'
    inline int fontPointSize() const {
        return std::max(1, m_theme.font.pointSize());
    }
    void setFontPointSize(int pt);

    // Visible grid metrics
    inline int visibleCols() const {
        return m_cols;
    }
    inline int visibleRows() const {
        return m_rows;
    }

signals:
    void focusDownRequested();

public slots:
    void onCursorChanged(mdn::Coord c);
    void onSelectionChanged(const mdn::Rect& r);
    void onModelModified();


protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* e) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    // Private member functions
    void recalcGridGeometry();
    void ensureCursorVisible();
    void drawAxes(QPainter& p, const QRect& widgetRect);
    void adjustFontBy(int deltaPts);

    // Private member data
    const mdn::Mdn2d* m_model{ nullptr };
    mdn::Selection* m_selection{ nullptr };
    class mdn::Project* m_project{ nullptr };

    Theme m_theme{};

    // Viewport
    int m_cellSize{20};
    int m_cols{32};
    int m_rows{16};
    int m_viewOriginX{0};
    int m_viewOriginY{0};

    // Cached for painting (not the source of truth)
    int m_cursorX{0};
    int m_cursorY{0};
    mdn::Rect m_cachedSel{};

    // Zoom bounds
    int m_minPt{8};
    int m_maxPt{48};
};

} // end namespace gui
} // end namespace mdn
