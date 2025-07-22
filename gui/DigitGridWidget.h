// DigitGridWidget.h
class DigitGridWidget : public QWidget {
    Q_OBJECT

public:
    DigitGridWidget(QWidget* parent = nullptr);
    void setMdn(std::shared_ptr<mdn::Mdn2d> mdn);
    void moveCursor(int dx, int dy);
    Coord getCursor() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    std::shared_ptr<mdn::Mdn2d> m_mdn;
    Coord m_cursor = Coord(0, 0);
    int m_cellSize = 20;
};
