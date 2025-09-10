#include "ProjectPropertiesDialog.hpp"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "../library/Mdn2dConfig.hpp"
#include "Project.hpp"
#include "../library/SignConvention.hpp"
#include "../library/Mdn2dConfigImpact.hpp"

using mdn::Mdn2dConfig;
using mdn::Mdn2dConfigImpact;
using mdn::gui::Project;

namespace mdn::gui {

ProjectPropertiesDialog::ProjectPropertiesDialog(Project* project, QWidget* parent)
    : QDialog(parent), m_project(project) {
    buildUi();
    setWindowTitle(tr("Project Properties"));
    setModal(true);
}


void ProjectPropertiesDialog::buildUi() {
    auto* outer = new QVBoxLayout(this);

    // Project group
    auto* gProject = new QGroupBox(tr("Project"), this);
    auto* formP = new QFormLayout(gProject);
    m_nameEdit = new QLineEdit(gProject);
    m_pathEdit = new QLineEdit(gProject);
    m_pathEdit->setReadOnly(true);
    formP->addRow(tr("Name:"), m_nameEdit);
    formP->addRow(tr("Path:"), m_pathEdit);
    outer->addWidget(gProject);

    // Global settings
    auto* gNum = new QGroupBox(tr("Global Number Settings"), this);
    auto* formN = new QFormLayout(gNum);

    m_base = new QSpinBox(gNum);
    m_base->setRange(2, 32);
    m_base->setAccelerated(true);

    m_precision = new QSpinBox(gNum);
    m_precision->setRange(2, std::numeric_limits<int>::max());
    m_precision->setAccelerated(true);

    // Fraxis
    auto* fraxisRow = new QWidget(gNum);
    auto* fraxisLay = new QHBoxLayout(fraxisRow);
    fraxisLay->setContentsMargins(0,0,0,0);
    m_fraxisX = new QRadioButton(tr("X"), fraxisRow);
    m_fraxisY = new QRadioButton(tr("Y"), fraxisRow);
    fraxisLay->addWidget(m_fraxisX);
    fraxisLay->addWidget(m_fraxisY);

    // Sign convention
    auto* signRow = new QWidget(gNum);
    auto* signLay = new QHBoxLayout(signRow);
    signLay->setContentsMargins(0,0,0,0);
    m_signPos = new QRadioButton(tr("Positive"), signRow);
    m_signNeutral = new QRadioButton(tr("Neutral"), signRow);
    m_signNeg = new QRadioButton(tr("Negative"), signRow);
    signLay->addWidget(m_signPos);
    signLay->addWidget(m_signNeutral);
    signLay->addWidget(m_signNeg);

    formN->addRow(tr("Base:"), m_base);
    formN->addRow(tr("Precision:"), m_precision);
    formN->addRow(tr("Fraxis:"), fraxisRow);
    formN->addRow(tr("Sign:"), signRow);
    outer->addWidget(gNum);

    // Impact preview
    auto* gImpact = new QGroupBox(tr("Change impact preview"), this);
    auto* v = new QVBoxLayout(gImpact);
    m_impactHeader = new QLabel(tr("—"), gImpact);
    m_impactHeader->setStyleSheet("font-weight:600;");
    m_impactBody   = new QLabel(tr("Adjust fields to see impact."), gImpact);
    m_impactBody->setWordWrap(true);
    v->addWidget(m_impactHeader);
    v->addWidget(m_impactBody);
    outer->addWidget(gImpact);

    // Buttons row
    auto* row = new QWidget(this);
    auto* rowLay = new QHBoxLayout(row);
    rowLay->setContentsMargins(0,0,0,0);
    auto* reset = new QPushButton(tr("Reset to Defaults"), row);
    auto* help  = new QPushButton(tr("Learn more…"), row);
    rowLay->addWidget(reset);
    rowLay->addStretch(1);
    rowLay->addWidget(help);
    outer->addWidget(row);

    // Dialog buttons
    m_btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    outer->addWidget(m_btns);

    // Wiring
    connect(m_btns, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_btns, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto fieldChanged = [this]{
        onAnyFieldChanged();
    };
    connect(m_nameEdit, &QLineEdit::textChanged, this, fieldChanged);
    connect(m_base,     qOverload<int>(&QSpinBox::valueChanged), this, fieldChanged);
    connect(m_precision,qOverload<int>(&QSpinBox::valueChanged), this, fieldChanged);
    connect(m_fraxisX,  &QRadioButton::toggled, this, fieldChanged);
    connect(m_fraxisY,  &QRadioButton::toggled, this, fieldChanged);
    connect(m_signPos,  &QRadioButton::toggled, this, fieldChanged);
    connect(m_signNeutral,&QRadioButton::toggled,this, fieldChanged);
    connect(m_signNeg,  &QRadioButton::toggled, this, fieldChanged);

    connect(reset, &QPushButton::clicked, this, &ProjectPropertiesDialog::onResetDefaults);
    connect(help,  &QPushButton::clicked, this, &ProjectPropertiesDialog::onLearnMore);
}


void ProjectPropertiesDialog::setInitial(const QString& pname,
                                         const QString& pathHint,
                                         const Mdn2dConfig& cfg) {
    m_nameEdit->setText(pname);
    m_pathEdit->setText(pathHint);

    // Cache defaults from cfg
    m_defBase      = cfg.base();
    m_defPrecision = cfg.precision();
    m_defFraxis    = (cfg.fraxis() == mdn::Fraxis::X) ? 0 : 1;
    int sc = 0;
    switch (cfg.signConvention()) {
        case mdn::SignConvention::Positive: sc = 0; break;
        case mdn::SignConvention::Neutral:  sc = 1; break;
        case mdn::SignConvention::Negative: sc = 2; break;
    }
    m_defSign = sc;

    // Set widgets to cfg
    m_base->setValue(m_defBase);
    m_precision->setValue(m_defPrecision);
    (m_defFraxis == 0 ? m_fraxisX : m_fraxisY)->setChecked(true);
    (m_defSign == 0 ? m_signPos : (m_defSign==1 ? m_signNeutral : m_signNeg))->setChecked(true);

    refreshImpactLabel();
}


QString ProjectPropertiesDialog::projectName() const { return m_nameEdit->text(); }


Mdn2dConfig ProjectPropertiesDialog::chosenConfig() const {
    int baseIn = m_base->value();
    int precisionIn = m_precision->value();
    Fraxis fraxisIn = m_fraxisX->isChecked() ? Fraxis::X : Fraxis::Y;
    SignConvention signConventionIn;
    if (m_signPos->isChecked()) {
        signConventionIn = SignConvention::Positive;
    } else if (m_signNeutral->isChecked()) {
        signConventionIn = SignConvention::Neutral;
    } else {
        signConventionIn = SignConvention::Negative;
    }
    Mdn2dConfig c(
        baseIn,
        precisionIn,
        signConventionIn,
        20, // default maxCarryOverIters
        fraxisIn
    );
    return c;
}


void ProjectPropertiesDialog::onAnyFieldChanged() {
    refreshImpactLabel();
}


void ProjectPropertiesDialog::refreshImpactLabel() {
    if (!m_project) return;

    const Mdn2dConfig cfg = chosenConfig();
    const Mdn2dConfigImpact imp = m_project->assessConfigChange(cfg);

    const QString name = QString::fromStdString(Mdn2dConfigImpactToName(imp));
    const QString desc = QString::fromStdString(Mdn2dConfigImpactToDescription(imp));

    m_impactHeader->setText(name);
    m_impactBody->setText(desc);
}


void ProjectPropertiesDialog::onResetDefaults() {
    m_base->setValue(m_defBase);
    m_precision->setValue(m_defPrecision);
    m_fraxisX->setChecked(m_defFraxis == 0);
    m_fraxisY->setChecked(m_defFraxis == 1);
    m_signPos->setChecked(m_defSign == 0);
    m_signNeutral->setChecked(m_defSign == 1);
    m_signNeg->setChecked(m_defSign == 2);
    refreshImpactLabel();
}


void ProjectPropertiesDialog::onLearnMore() {
    QMessageBox::information(this, tr("Number settings"),
        tr("• Base: numeral base (2–32). Changing base may clear all digits.\n"
           "• Precision: limits the non-zero digit envelope; excess digits drop at the far edge.\n"
           "• Fraxis: controls which axis the fractional part ‘fans out’ along (X or Y).\n"
           "• Sign: resolves ambiguous carryovers (Positive/Neutral/Negative)."));
}


} // namespace mdn::gui
