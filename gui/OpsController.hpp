#pragma once
#include <QObject>
#include <QString>

#include "EnumOperation.hpp"
#include "EnumOperationPhase.hpp"
#include "EnumDestinationMode.hpp"

namespace mdn {
namespace gui {

// Forward declarations
class Project;
class HoverPeekTabWidget;


class OpsController : public QObject {
    Q_OBJECT
public:
    explicit OpsController(Project* proj, HoverPeekTabWidget* tabs, QObject* parent=nullptr);


signals:
    // show text like "Mdn1 × *Choose*"
    void requestStatus(const QString& msg);
    // enable/disable Cancel button
    void requestCancelEnabled(bool on);
    // MainWindow can create a tab and select it
    void requestNewTab(const QString& name);

public slots:
    // Buttons
    void startAdd(); void startSub(); void startMul(); void startDiv();
    void cancel();

    // From tabs
    void onTabCommitted(int idx);

    // From [NewTab]
    void onNewTab();

private:
    // enter PickB
    void begin(Operation op);
    // perform the op
    void finishTo(int destIndex, bool isNew);

    // helper for status text
    QString tabName(int idx) const;

private:
    Project* m_project{};
    HoverPeekTabWidget* m_tabs{};
    Phase m_phase{Phase::Idle};
    Operation m_op{Operation::Add};
    int m_a{-1}, m_b{-1};
};

} // end namespace gui
} // end namespace mdn
