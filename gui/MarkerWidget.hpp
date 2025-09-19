// MarkerWidget.h
#pragma once
#include <QWidget>

namespace mdn {
namespace gui {

class MarkerWidget final : public QWidget
{
    Q_OBJECT
public:
    explicit MarkerWidget(QWidget* parent = nullptr) : QWidget(parent) {}
};

} // end namespace gui
} // end namespace mdn
