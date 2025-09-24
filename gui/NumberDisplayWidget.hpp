#pragma once

#include <QBrush>
#include <QFont>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>
#include <QPen>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QWidget>

#include "../library/GlobalConfig.hpp"
#include "../library/Mdn2d.hpp"
#include "../library/Coord.hpp"
#include "../library/Rect.hpp"
#include "../library/Digit.hpp"

// Forward declarations
namespace mdn {
class Mdn2d;
class Selection;
namespace gui {
class Project;
} // end namespace gui forward declaration
} // end namespace mdn forward declaration

namespace mdn {
namespace gui {

enum class HighlightRole { None, Peek };


class NumberDisplayWidget : public QWidget {
    Q_OBJECT

public:
    enum class EditMode {
        Overwrite,
        Add,
        Subtract
    };
    Q_ENUM(EditMode)
    static std::string EditModeToString(EditMode m);

    struct Theme {
        QFont   font            {QStringLiteral("Courier New"), 11};
        QBrush  background      {Qt::NoBrush};
        QPen    axisPen         {QColor(128,128,128), 3};
        QPen    originPen       {QColor(220,0,0), 2};
        // Grid & text pens for out-of-bounds (zeroes) part of window
        QPen    gridPen         {QColor(220,220,220), 1};
        QPen    textPen         {QColor(120,120,120)};
        // Grid & text pens for in-bounds (non-zero) part of window
        QPen    nzGridPen       {QColor(160,160,160), 1};
        QPen    nzTextPen       {QColor(10,10,10)};
        QBrush  selectionFill   {QColor(82,143,255,70)};
        QBrush  anchorFill      {QColor(128,128,128,90)};
        QBrush  cursorFill      {QColor(255,214,64,140)};
    };


    explicit NumberDisplayWidget(QWidget* parent = nullptr);

    // Bindings
    void setProject(Project* proj);
    void setModel(Mdn2d* mdn, Selection* sel);
    Mdn2d* model() { return m_model; }
    const Mdn2d* model() const { return m_model; }

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

    // Cell editting
    void cancelCellEdit();

    inline EditMode editMode() const { return m_mode; }


signals:
    void focusDownRequested();
    void requestCycleEditMode(bool forward);
    void requestToggleEditMode(EditMode m);
    void requestSetEditMode(EditMode m);
    void requestCycleFraxis();
    void requestStatus(const QString& s, int timeOut);

Q_SIGNALS:
    void requestSelectNextTab();
    void requestSelectPrevTab();
    void requestMoveTabRight();
    void requestMoveTabLeft();
    void requestDebugShowAllTabs();
    void statusCursorChanged(int x, int y);
    void statusSelectionChanged(const mdn::Selection& s);
    void editModeChanged(mdn::gui::NumberDisplayWidget::EditMode m);
    void requestFontSizeChange(int pt);

public slots:
    void onCursorChanged(mdn::Coord c);
    void onSelectionChanged(const mdn::Rect& r);
    void onModelModified();
    void setEditMode(EditMode m);
    void setHighlightRole(HighlightRole r);
    HighlightRole highlightRole() const { return m_highlightRole; }


protected:
    void paintEvent(QPaintEvent*) override;

    bool eventFilter(QObject* watched, QEvent* event) override;

    // Acts as selector to the two keyPressEvent functions below
    void keyPressEvent(QKeyEvent*) override;
        // Selected when not editing a digit
        void keyPressEvent_gridScope(QKeyEvent*);
        // Selected when editing a digit
        void keyPressEvent_digitEditScope(QKeyEvent*);

    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent* e) override;
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool focusNextPrevChild(bool) override { return false; }

private:

    // Private member types

    // Submission directions
    enum class SubmitMove {
        Enter,
        ShiftEnter,
        Tab,
        ShiftTab
    };


    // Private member functions
    void recalcGridGeometry();
    void ensureCursorVisible();
    void drawAxes(QPainter& p, const QRect& widgetRect);
    void adjustFontBy(int deltaPts);
    void pixelToModel(int px, int py, int& mx, int& my) const;
public:
    void centreViewOn(int mx, int my);
    void centreViewOnOrigin();
    void armCentreViewOnOrigin();
    void selectAllBounds();
private:
    void captureCursorFractions();
    void restoreCursorFractions();
    void setBothCursors(int mx, int my);
    void setCursor1(int mx, int my);
    void clearSelection();
    QString modeShortText() const;
    QString selectionSummaryText() const;
    void postNavRefresh();

    // API
    void beginCellEdit(const QString& initialText);
    void positionCellEditor();
    // how: direction to move, stayInside: if there's a selection rectangle, true=stay inside it
    void commitCellEdit(SubmitMove how, bool stayInside);

    // Helpers
    bool isGridTypingKey(const QKeyEvent* ev) const;
    bool textAcceptableForBase(const QString& s, int base) const;
    double parseBaseReal(const QString& s, int base, bool& ok) const;
    int charToDigitValue(QChar ch) const;
    QRect cursorCellRectInWidget() const;
    // how: direction to move, stayInside: if there's a selection rectangle, true=stay inside it
    void moveCursorAfterSubmit(SubmitMove how, bool stayInside);
    QString stripSign(const QString& s, bool& isNeg) const;
    double parseBaseRealMagnitude(
        const QString& body,
        int base,
        int& nFracDigitsOut,
        bool& ok
    ) const;
    long long parseBaseIntMagnitude(const QString& body, int base, bool& ok) const;

    // Private member data
    Mdn2d* m_model{ nullptr };
    Selection* m_selection{ nullptr };
    Project* m_project{ nullptr };

    Theme m_theme{};

    // Viewport
    int m_cellSize{20};
    int m_cols{32};
    int m_rows{16};
    int m_viewOriginX{0};
    int m_viewOriginY{0};
    Rect m_viewBounds;

    // Cached for painting (not the source of truth)
    int m_cursorX{0};
    int m_cursorY{0};

    // Zoom bounds
    int m_minPt{8};
    int m_maxPt{48};

    static constexpr double kEdgeGuardFrac = 0.10;
    double m_lastCursorFracX{0.5};
    double m_lastCursorFracY{0.5};
    bool m_armCentreViewOnOrigin{false};

    bool m_dragging{false};
    int m_dragAnchorX{0};
    int m_dragAnchorY{0};

    bool m_deferPostRestore{ false };

    // In-cell editor
    QLineEdit* m_cellEditor{nullptr};
    EditMode m_mode{ EditMode::Overwrite };

    // Editing state
    bool m_editing{false};
    HighlightRole m_highlightRole = HighlightRole::None;

};

} // end namespace gui
} // end namespace mdn
