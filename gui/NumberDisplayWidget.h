#pragma once

#include <qnamespace.h>
#include <QWidget>
// #include <QtGui/qcolor.h>
// #include <QColor>

#include "../library/Mdn2d.h"

class NumberDisplayWidget : public QWidget {
    Q_OBJECT

public:
    NumberDisplayWidget(QWidget* parent = nullptr);

    void setModel(const mdn::Mdn2d* mdn);
    void setViewCenter(int x, int y);
    void moveCursor(int dx, int dy);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    const mdn::Mdn2d* m_model = nullptr;

    Qt::GlobalColor m_defaultColors_gridLines = Qt::gray;

    // X coordinate, in digit position, of top-left cell in view
    int m_viewOriginX = -16;

    // Y coordinate, in digit position, of top-left cell in view
    int m_viewOriginY = -16;

    // X digit coordinate of the cursor location
    int m_cursorX = 0;

    // Y digit coordinate of the cursor location
    int m_cursorY = 0;

    // Number of columns in view
    int m_cols = 32;

    // Number of rows in view
    int m_rows = 32;

    // Size of each cell in pixels
    int m_cellSize = 20;


};
