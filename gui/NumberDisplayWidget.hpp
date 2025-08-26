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

// Forward declarations
namespace mdn {
class Mdn2d;
namespace gui {
class Selection;
class Project;
} // end namespace gui forward declaration
} // end namespace mdn forward declaration

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

    // Emitted when this widget gains focus
    void focused(NumberDisplayWidget* self);

public slots:
    // Centre the view on the current cursor position
    void centreOnCursor();

    // Centre the view on the origin
    void centreOnOrigin();

    // Increase the font point size and keep the cursor at the same on-screen fraction
    void zoomIn();

    // Decrease the font point size and keep the cursor at the same on-screen fraction
    void zoomOut();

    // Reset the font point size to a sensible default and keep the cursor fraction
    void zoomReset();
    void onCursorChanged(mdn::Coord c);
    void onSelectionChanged(const mdn::Rect& r);
    void onModelModified();


protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent* e) override;
    void resizeEvent(QResizeEvent* event) override;
    void focusInEvent(QFocusEvent*) override;

private:
    // Private member functions

    // Recompute grid geometry and page steps
    void recalcGridGeometry();

    // Ensure cursor obeys edge guard
    void ensureCursorVisible();

    void drawAxes(QPainter& p, const QRect& widgetRect);
    void adjustFontBy(int deltaPts);

    // Map widget pixel to model coordinate
    void pixelToModel(int px, int py, int& mx, int& my) const;

    // Centre the view on a model coordinate
    void centreViewOn(int mx, int my);

    // Centre the view on the origin (0,0)
    void centreViewOnOrigin();

    // Update last known fractional cursor position within viewport
    void captureCursorFractions();

    // Restore view origin so cursor appears at stored fractional position
    void restoreCursorFractions();

    // Set both cursor0 and cursor1 to one point
    void setBothCursors(int mx, int my);

    // Set only cursor1 (extend selection)
    void setCursor1(int mx, int my);

    // Change the font point size by a delta and preserve cursor fraction
    void adjustZoomBySteps(int deltaSteps);

    // Set the font point size absolutely and preserve cursor fraction
    void setZoomPointSize(int pt);


    // Private member data
    const mdn::Mdn2d* m_model{ nullptr };
    Selection* m_selection{ nullptr };
    Project* m_project{ nullptr };

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

    static constexpr double kEdgeGuardFrac = 0.10;
    double m_lastCursorFracX{0.5};
    double m_lastCursorFracY{0.5};

    bool m_dragging{false};
    int m_dragAnchorX{0};
    int m_dragAnchorY{0};

};

} // end namespace gui
} // end namespace mdn
