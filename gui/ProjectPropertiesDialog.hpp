#pragma once
#include <QDialog>

class QCheckBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSpinBox;

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
    void applyConfig(const Mdn2dConfig& model);


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
    QSpinBox*  m_fraxisCascadeDepth{nullptr};
    QSpinBox*  m_precision{nullptr};
    QCheckBox*  m_precisionUnlimited{nullptr};
    QRadioButton* m_fraxisX{nullptr};
    QRadioButton* m_fraxisY{nullptr};
    QRadioButton* m_signPos{nullptr};
    QRadioButton* m_signNeutral{nullptr};
    QRadioButton* m_signNeg{nullptr};

    QLabel* m_impactHeader{nullptr};
    QLabel* m_impactBody{nullptr};
    QDialogButtonBox* m_btns{nullptr};

    // cache of initial defaults (from cfg)
    int m_defBase{10};
    int m_defFraxisCascadeDepth{20};
    int m_defPrecision{32};
    bool m_defPrecisionUnlimited{true};
    int m_defFraxis{0};  // 0=X, 1=Y
    int m_defSign{0};    // 0=Positive, 1=Neutral, 2=Negative
};

} // namespace mdn::gui
