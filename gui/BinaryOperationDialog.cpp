#include "BinaryOperationDialog.hpp"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QCheckBox>

mdn::gui::BinaryOperationDialog::BinaryOperationDialog(QWidget* parent)
:
    QDialog(parent)
{
    buildUi();
}


void mdn::gui::BinaryOperationDialog::setTabNames(const QStringList& names) {
    m_names = names;
    rebuildBPicker();
    updateSummary();
}


void mdn::gui::BinaryOperationDialog::setActiveIndex(int indexA) {
    m_indexA = indexA;
    if (m_labelA) {
        m_labelA->setText(m_names.value(m_indexA));
    }
    updateSummary();
}


void mdn::gui::BinaryOperationDialog::setInitialOperation(Operation op) {
    m_op = op;
    if (m_opsButtons) {
        int id = static_cast<int>(op);
        QAbstractButton* b = m_opsButtons->button(id);
        if (b) {
            b->setChecked(true);
        }
    }
    updateSummary();
}


void mdn::gui::BinaryOperationDialog::setRememberedB(int indexB) {
    m_indexB = indexB;
    selectBInUi(m_indexB);
    updateSummary();
}


void mdn::gui::BinaryOperationDialog::setRememberedDestination(DestinationMode mode) {
    m_dest = mode;
    if (mode == DestinationMode::OverwriteA) {
        m_destOverwriteA->setChecked(true);
    } else {
        if (mode == DestinationMode::OverwriteB) {
            m_destOverwriteB->setChecked(true);
        } else {
            m_destCreateNew->setChecked(true);
        }
    }
    updateSummary();
}


mdn::gui::BinaryOperationDialog::Plan mdn::gui::BinaryOperationDialog::plan() const {
    BinaryOperationDialog::Plan p;
    p.op = m_op;
    p.indexA = m_indexA;
    p.indexB = m_indexB;
    p.dest = m_dest;
    p.newName = m_newNameEdit->text();
    p.rememberChoices = m_remember->isChecked();
    return p;
}


void mdn::gui::BinaryOperationDialog::onOpChanged() {
    int id = m_opsButtons->checkedId();
    if (id < 0) {
        return;
    }
    m_op = static_cast<Operation>(id);
    updateSummary();
}


void mdn::gui::BinaryOperationDialog::onBSelectionChanged() {
    m_indexB = currentBFromUi();
    updateSummary();
}


void mdn::gui::BinaryOperationDialog::onDestChanged() {
    if (m_destOverwriteA->isChecked()) {
        m_dest = DestinationMode::OverwriteA;
    } else {
        if (m_destOverwriteB->isChecked()) {
            m_dest = DestinationMode::OverwriteB;
        } else {
            m_dest = DestinationMode::CreateNew;
        }
    }
    bool needName = m_dest == DestinationMode::CreateNew;
    m_newNameEdit->setEnabled(needName);
    if (needName) {
        QString a = m_names.value(m_indexA);
        QString b = m_names.value(m_indexB);
        QString sym = opSymbol(m_op);
        m_newNameEdit->setText(a + " " + sym + " " + b);
    }
    updateSummary();
}


void mdn::gui::BinaryOperationDialog::onFilterTextChanged(const QString& text) {
    for (int i = 0; i < m_bList->count(); ++i) {
        QListWidgetItem* it = m_bList->item(i);
        bool show = it->text().contains(text, Qt::CaseInsensitive);
        it->setHidden(!show);
    }
}


void mdn::gui::BinaryOperationDialog::onAccept() {
    accept();
}


void mdn::gui::BinaryOperationDialog::buildUi() {
    QVBoxLayout* root = new QVBoxLayout(this);

    QHBoxLayout* opsRow = new QHBoxLayout();
    m_opsButtons = new QButtonGroup(this);

    QRadioButton* add = new QRadioButton(tr("Add"));
    QRadioButton* sub = new QRadioButton(tr("Subtract"));
    QRadioButton* mul = new QRadioButton(tr("Multiply"));
    QRadioButton* div = new QRadioButton(tr("Divide"));

    m_opsButtons->addButton(add, static_cast<int>(Operation::Add));
    m_opsButtons->addButton(sub, static_cast<int>(Operation::Subtract));
    m_opsButtons->addButton(mul, static_cast<int>(Operation::Multiply));
    m_opsButtons->addButton(div, static_cast<int>(Operation::Divide));

    add->setChecked(true);

    opsRow->addWidget(add);
    opsRow->addWidget(sub);
    opsRow->addWidget(mul);
    opsRow->addWidget(div);
    QWidget* opsRowW = new QWidget(this);
    opsRowW->setLayout(opsRow);

    connect(m_opsButtons, SIGNAL(buttonClicked(int)), this, SLOT(onOpChanged()));

    QHBoxLayout* aRow = new QHBoxLayout();
    QLabel* aLabel = new QLabel(tr("A (active):"));
    m_labelA = new QLabel(tr(""));
    aRow->addWidget(aLabel);
    aRow->addWidget(m_labelA, 1);
    QWidget* aRowW = new QWidget(this);
    aRowW->setLayout(aRow);

    QVBoxLayout* bBox = new QVBoxLayout();
    QLabel* bLabel = new QLabel(tr("Operand B:"));
    bBox->addWidget(bLabel);

    m_bPickerStack = new QWidget(this);
    QVBoxLayout* bStackLayout = new QVBoxLayout(m_bPickerStack);

    m_bRadioGroup = new QGroupBox(tr("Choose B"), this);
    m_bRadioButtons = new QButtonGroup(this);
    QVBoxLayout* radioLay = new QVBoxLayout(m_bRadioGroup);
    m_bRadioGroup->setLayout(radioLay);

    m_bList = new QListWidget(this);
    m_bFilter = new QLineEdit(this);

    m_bCombo = new QComboBox(this);
    m_bCombo->setEditable(true);

    bStackLayout->addWidget(m_bRadioGroup);
    bStackLayout->addWidget(m_bFilter);
    bStackLayout->addWidget(m_bList);
    bStackLayout->addWidget(m_bCombo);

    bBox->addWidget(m_bPickerStack);
    QWidget* bPicker = new QWidget(this);
    bPicker->setLayout(bBox);

    connect(m_bRadioButtons, SIGNAL(buttonClicked(int)), this, SLOT(onBSelectionChanged()));
    connect(m_bList, SIGNAL(currentRowChanged(int)), this, SLOT(onBSelectionChanged()));
    connect(m_bCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onBSelectionChanged()));
    connect(m_bFilter, SIGNAL(textChanged(QString)), this, SLOT(onFilterTextChanged(QString)));

    QGroupBox* destGroup = new QGroupBox(tr("Destination"), this);
    QVBoxLayout* destLay = new QVBoxLayout(destGroup);

    m_destOverwriteA = new QRadioButton(tr("Overwrite A"), destGroup);
    m_destOverwriteB = new QRadioButton(tr("Overwrite B"), destGroup);
    m_destCreateNew  = new QRadioButton(tr("Create new"),  destGroup);

    m_newNameEdit = new QLineEdit(destGroup);
    m_newNameEdit->setEnabled(false);

    destLay->addWidget(m_destOverwriteA);
    destLay->addWidget(m_destOverwriteB);

    QHBoxLayout* nameRow = new QHBoxLayout();
    nameRow->addWidget(m_destCreateNew);
    nameRow->addWidget(new QLabel(tr("Name:"), destGroup));
    nameRow->addWidget(m_newNameEdit, 1);
    QWidget* nameRowW = new QWidget(destGroup);
    nameRowW->setLayout(nameRow);
    destLay->addWidget(nameRowW);

    auto* destButtons = new QButtonGroup(destGroup);
    destButtons->setExclusive(true);
    destButtons->addButton(m_destOverwriteA);
    destButtons->addButton(m_destOverwriteB);
    destButtons->addButton(m_destCreateNew);

    connect(m_destOverwriteA, SIGNAL(toggled(bool)), this, SLOT(onDestChanged()));
    connect(m_destOverwriteB, SIGNAL(toggled(bool)), this, SLOT(onDestChanged()));
    connect(m_destCreateNew, SIGNAL(toggled(bool)), this, SLOT(onDestChanged()));

    m_summary = new QLabel(tr(""), this);
    m_remember = new QCheckBox(tr("Remember B and destination"), this);

    m_buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, this);
    connect(m_buttons, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));

    root->addWidget(opsRowW);
    root->addWidget(aRowW);
    root->addWidget(bPicker);
    root->addWidget(destGroup);
    root->addWidget(m_summary);
    root->addWidget(m_remember);
    root->addWidget(m_buttons);

    setWindowTitle(tr("Binary Operation"));
    resize(560, 560);
}


void mdn::gui::BinaryOperationDialog::rebuildBPicker() {
    if (m_names.size() <= 7) {
        m_bRadioGroup->setVisible(true);
        m_bFilter->setVisible(false);
        m_bList->setVisible(false);
        m_bCombo->setVisible(false);

        QList<QAbstractButton*> old = m_bRadioButtons->buttons();
        for (QAbstractButton* b : old) {
            m_bRadioButtons->removeButton(b);
            b->deleteLater();
        }
        QVBoxLayout* radioLay = static_cast<QVBoxLayout*>(m_bRadioGroup->layout());
        while (QLayoutItem* it = radioLay->takeAt(0)) {
            if (it->widget()) {
                it->widget()->deleteLater();
            }
            delete it;
        }
        for (int i = 0; i < m_names.size(); ++i) {
            QRadioButton* rb = new QRadioButton(m_names.at(i), m_bRadioGroup);
            m_bRadioButtons->addButton(rb, i);
            radioLay->addWidget(rb);
        }
        if (m_names.size() > 0) {
            QAbstractButton* first = m_bRadioButtons->button(0);
            if (first) {
                first->setChecked(true);
            }
        }
    } else {
        if (m_names.size() <= 25) {
            m_bRadioGroup->setVisible(false);
            m_bFilter->setVisible(true);
            m_bList->setVisible(true);
            m_bCombo->setVisible(false);

            m_bList->clear();
            for (const QString& n : m_names) {
                m_bList->addItem(n);
            }
            if (m_names.size() > 0) {
                m_bList->setCurrentRow(0);
            }
        } else {
            m_bRadioGroup->setVisible(false);
            m_bFilter->setVisible(false);
            m_bList->setVisible(false);
            m_bCombo->setVisible(true);

            m_bCombo->clear();
            m_bCombo->addItems(m_names);
            if (m_names.size() > 0) {
                m_bCombo->setCurrentIndex(0);
            }
        }
    }

    selectBInUi(m_indexB);
}


void mdn::gui::BinaryOperationDialog::selectBInUi(int indexB) {
    if (m_names.isEmpty()) {
        m_indexB = 0;
        return;
    }
    int idx = indexB;
    if (idx < 0) {
        idx = 0;
    } else {
        if (idx >= m_names.size()) {
            idx = m_names.size() - 1;
        }
    }
    if (m_bRadioGroup->isVisible()) {
        QAbstractButton* b = m_bRadioButtons->button(idx);
        if (b) {
            b->setChecked(true);
        }
    } else {
        if (m_bList->isVisible()) {
            m_bList->setCurrentRow(idx);
        } else {
            m_bCombo->setCurrentIndex(idx);
        }
    }
    m_indexB = idx;
}


int mdn::gui::BinaryOperationDialog::currentBFromUi() const {
    if (m_bRadioGroup->isVisible()) {
        int id = m_bRadioButtons->checkedId();
        return id;
    } else {
        if (m_bList->isVisible()) {
            return m_bList->currentRow();
        } else {
            return m_bCombo->currentIndex();
        }
    }
}


void mdn::gui::BinaryOperationDialog::updateSummary() {
    QString a = m_names.value(m_indexA);
    QString b = m_names.value(m_indexB);
    QString sym = opSymbol(m_op);

    QString destStr;
    if (m_dest == DestinationMode::OverwriteA) {
        destStr = QStringLiteral("Overwrite %1").arg(a);
    } else {
        if (m_dest == DestinationMode::OverwriteB) {
            destStr = QStringLiteral("Overwrite %1").arg(b);
        } else {
            destStr = QStringLiteral("Create \"%1\"").arg(m_newNameEdit->text());
        }
    }

    QString s = QStringLiteral("%1  %2  %3  →  %4").arg(a, sym, b, destStr);
    m_summary->setText(s);
}


QString mdn::gui::BinaryOperationDialog::opSymbol(Operation op) const {
    if (op == Operation::Add) {
        return "+";
    } else {
        if (op == Operation::Subtract) {
            return "-";
        } else {
            if (op == Operation::Multiply) {
                return "×";
            } else {
                return "÷";
            }
        }
    }
}


QString mdn::gui::BinaryOperationDialog::opName(Operation op) const {
    if (op == Operation::Add) {
        return tr("Add");
    } else {
        if (op == Operation::Subtract) {
            return tr("Subtract");
        } else {
            if (op == Operation::Multiply) {
                return tr("Multiply");
            } else {
                return tr("Divide");
            }
        }
    }
}


QString mdn::gui::BinaryOperationDialog::makeUnique(
    const QString& base, const QStringList& existing
) const {
    if (!existing.contains(base, Qt::CaseSensitive)) return base;
    int n = 1;
    for (;;) {
        const QString trial = base + QStringLiteral("_%1").arg(n);
        if (!existing.contains(trial, Qt::CaseSensitive)) return trial;
        ++n;
    }
}