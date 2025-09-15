#pragma once
#include <QDialog>

class QSpinBox;
class QLineEdit;
class QLabel;
class QRadioButton;
class QDialogButtonBox;

namespace mdn {
    class Mdn2dConfig;
}
namespace mdn::gui {
class Project; // forward

class ProjectPropertiesDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProjectPropertiesDialog(Project* project=nullptr, QWidget* parent=nullptr);

    void setInitial(
        const QString& projectName,
        const QString& pathHint,
        const mdn::Mdn2dConfig& cfg
    );

    QString projectName() const;
    mdn::Mdn2dConfig chosenConfig() const;

private slots:
    void onAnyFieldChanged();
    void onResetDefaults();
    void onLearnMore();

private:
    void buildUi();
    void refreshImpactLabel();

private:
    Project* m_project{nullptr};

    QLineEdit* m_nameEdit{nullptr};
    QLineEdit* m_pathEdit{nullptr};

    QSpinBox*  m_base{nullptr};
    QSpinBox*  m_precision{nullptr};
    QRadioButton* m_fraxisX{nullptr};
    QRadioButton* m_fraxisY{nullptr};
    QRadioButton* m_signPos{nullptr};
    QRadioButton* m_signNeutral{nullptr};
    QRadioButton* m_signNeg{nullptr};

    QLabel* m_impactHeader{nullptr};
    QLabel* m_impactBody{nullptr};
    QDialogButtonBox* m_btns{nullptr};

    // cache of initial defaults (from cfg)
    int m_defBase{10}, m_defPrecision{32};
    int m_defFraxis{0};  // 0=X, 1=Y
    int m_defSign{0};    // 0=Positive, 1=Neutral, 2=Negative
};

} // namespace mdn::gui
