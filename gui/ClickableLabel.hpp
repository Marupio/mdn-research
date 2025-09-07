#pragma once

#include <QEvent>
#include <QLabel>
#include <QMouseEvent>

class ClickableLabel : public QLabel {
  Q_OBJECT
public:
  using QLabel::QLabel;
signals:
  void clicked(Qt::MouseButton);
protected:
  void mousePressEvent(QMouseEvent* e) override {
    emit clicked(e->button());
    QLabel::mousePressEvent(e);
  }
};
