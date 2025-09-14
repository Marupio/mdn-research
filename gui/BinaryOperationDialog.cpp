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
    Log_Debug3_H("");
    buildUi();
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::setTabNames(const QStringList& names) {
    Log_Debug3_H("");
    m_names = names;
    rebuildBPicker();
    rebuildDestPicker();
    updateSummary();
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::setActiveIndex(int indexA) {
    Log_Debug3_H("");
    m_indexA = indexA;
    if (m_labelA) {
        m_labelA->setText(m_names.value(m_indexA));
    }
    // Default dest selection follows active A unless user changes it
    if (m_indexDest < 0) {
        m_indexDest = m_indexA;
        selectDestInUi(m_indexDest);
    }
    rebuildDestPicker();
    updateSummary();
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::setInitialOperation(Operation op) {
    Log_Debug3_H("");
    m_op = op;
    if (m_opsButtons) {
        int id = static_cast<int>(op);
        QAbstractButton* b = m_opsButtons->button(id);
        if (b) {
            b->setChecked(true);
        }
    }
    updateSummary();
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::setRememberedB(int indexB) {
    Log_Debug3_H("");
    m_indexB = indexB;
    selectBInUi(m_indexB);
    Log_Debug3_H("mIndexB=" << m_indexB);
    updateSummary();
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::setRememberedDestination(DestinationMode mode) {
    Log_Debug3_H("");
    m_dest = mode;
    if (mode == DestinationMode::CreateNew) {
        m_destCreateNew->setChecked(true);
    } else {
        m_destOverwrite->setChecked(true);
    }
    if (m_indexDest < 0) {
        m_indexDest = m_indexA;
        selectDestInUi(m_indexDest);
    }
    updateSummary();
    Log_Debug3_T("");
}


mdn::gui::BinaryOperationDialog::Plan mdn::gui::BinaryOperationDialog::plan() const {
    Log_Debug3_H("");
    BinaryOperationDialog::Plan p;
    p.op = m_op;
    p.indexA = m_indexA;
    p.indexB = m_indexB;
    p.dest = m_dest;
    p.indexDest = (m_dest == DestinationMode::OverwriteIndex ? currentDestFromUi() : -1);
    p.newName = m_newNameEdit->text();
    p.rememberChoices = m_remember->isChecked();
    Log_Debug3_T("returning p=" << p);
    return p;
}


void mdn::gui::BinaryOperationDialog::onOpChanged() {
    Log_Debug3_H("");
    int id = m_opsButtons->checkedId();
    if (id < 0) {
        Log_Debug3_T("");
        return;
    }
    m_op = static_cast<Operation>(id);
    updateSummary();
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::onBSelectionChanged() {
    Log_Debug3_H("");
    m_indexB = currentBFromUi();
    Log_Debug3_H("mIndexB=" << m_indexB);
    updateSummary();
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::onDestChanged() {
    Log_Debug3_H("");
    if (m_destOverwrite->isChecked()) {
        m_dest = DestinationMode::OverwriteIndex;
    } else {
        m_dest = DestinationMode::CreateNew;
    }

    const bool needPicker = m_dest == DestinationMode::OverwriteIndex;
    const bool needName = m_dest == DestinationMode::CreateNew;

    m_destPickerStack->setEnabled(needPicker);
    m_destPickerStack->setVisible(needPicker);
    m_newNameEdit->setEnabled(needName);

    if (needName) {
        QString a = m_names.value(m_indexA);
        QString b = m_names.value(m_indexB);
        QString sym = opSymbol(m_op);
        m_newNameEdit->setText(a + " " + sym + " " + b);
    }
    if (needPicker && m_indexDest < 0) {
        m_indexDest = m_indexA;
        selectDestInUi(m_indexDest);
    }
    updateSummary();
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::onDestSelectionChanged() {
    Log_Debug3_H("");
    m_indexDest = currentDestFromUi();
    updateSummary();
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::onDestFilterTextChanged(const QString& text) {
    Log_Debug3_H("");
    for (int i = 0; i < m_destList->count(); ++i) {
        QListWidgetItem* it = m_destList->item(i);
        bool show = it->text().contains(text, Qt::CaseInsensitive);
        it->setHidden(!show);
    }
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::onBFilterTextChanged(const QString& text) {
    Log_Debug3_H("");
    for (int i = 0; i < m_bList->count(); ++i) {
        QListWidgetItem* it = m_bList->item(i);
        bool show = it->text().contains(text, Qt::CaseInsensitive);
        it->setHidden(!show);
    }
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::onAccept() {
    accept();
}


void mdn::gui::BinaryOperationDialog::buildUi() {
    Log_Debug3_H("");
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

    // connect(m_opsButtons, SIGNAL(buttonClicked(int)), this, SLOT(onOpChanged()));

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

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(m_opsButtons,      &QButtonGroup::idClicked,
            this,              &BinaryOperationDialog::onOpChanged);
    connect(m_bRadioButtons,   &QButtonGroup::idClicked,
            this,              &BinaryOperationDialog::onBSelectionChanged);
#else
    connect(m_opsButtons,    QOverload<int>::of(&QButtonGroup::buttonClicked),
            this,            &BinaryOperationDialog::onOpChanged);
    connect(m_bRadioButtons, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this,            &BinaryOperationDialog::onBSelectionChanged);
#endif

    connect(m_bList, SIGNAL(currentRowChanged(int)), this, SLOT(onBSelectionChanged()));
    connect(m_bCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onBSelectionChanged()));
    connect(m_bFilter, SIGNAL(textChanged(QString)), this, SLOT(onBFilterTextChanged(QString)));

    // Destination box
    QVBoxLayout* destBox = new QVBoxLayout();
    QLabel* destLabel = new QLabel(tr("Answer:"), this);
    destBox->addWidget(destLabel);

    // Overwrite/Create radios
    QWidget* destRadioRow = new QWidget(this);
    QHBoxLayout* destRadioLay = new QHBoxLayout(destRadioRow);

    m_destOverwrite = new QRadioButton(tr("Overwrite"), this);
    m_destCreateNew = new QRadioButton(tr("Create New"), this);
    m_destOverwrite->setChecked(true);

    destRadioLay->addWidget(m_destOverwrite);
    destRadioLay->addWidget(m_destCreateNew);
    destRadioRow->setLayout(destRadioLay);
    destBox->addWidget(destRadioRow);

    // Destination picker stack
    m_destPickerStack = new QWidget(this);
    QVBoxLayout* dStack = new QVBoxLayout(m_destPickerStack);

    m_destRadioGroup = new QGroupBox(tr("Choose destination"), this);
    m_destRadioButtons = new QButtonGroup(this);
    QVBoxLayout* dRadioLay = new QVBoxLayout(m_destRadioGroup);
    m_destRadioGroup->setLayout(dRadioLay);

    m_destFilter = new QLineEdit(this);
    m_destFilter->setPlaceholderText(tr("Filter…"));
    m_destList = new QListWidget(this);
    m_destCombo = new QComboBox(this);

    dStack->addWidget(m_destRadioGroup);
    dStack->addWidget(m_destFilter);
    dStack->addWidget(m_destList);
    dStack->addWidget(m_destCombo);
    m_destPickerStack->setLayout(dStack);
    destBox->addWidget(m_destPickerStack);

    // New name row
    QHBoxLayout* nameRow = new QHBoxLayout();
    QLabel* nameLabel = new QLabel(tr("New name:"), this);
    m_newNameEdit = new QLineEdit(this);
    nameRow->addWidget(nameLabel);
    nameRow->addWidget(m_newNameEdit, 1);
    QWidget* nameRowW = new QWidget(this);
    nameRowW->setLayout(nameRow);

    // Summary + buttons
    m_summary = new QLabel(this);
    QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    root->addWidget(opsRowW);
    root->addWidget(aRowW);
    root->addWidget(bLabel);
    root->addWidget(m_bPickerStack);
    root->addLayout(destBox);
    root->addWidget(nameRowW);
    root->addWidget(m_summary, 0);
    root->addWidget(btns, 0);
    setLayout(root);

    connect(m_opsButtons, SIGNAL(buttonClicked(int)), this, SLOT(onOpChanged()));
    connect(m_bRadioButtons, SIGNAL(buttonClicked(int)), this, SLOT(onBSelectionChanged()));
    connect(m_bList, SIGNAL(currentRowChanged(int)), this, SLOT(onBSelectionChanged()));
    connect(m_bCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onBSelectionChanged()));
    connect(m_destOverwrite, SIGNAL(toggled(bool)), this, SLOT(onDestChanged()));
    connect(m_destCreateNew, SIGNAL(toggled(bool)), this, SLOT(onDestChanged()));
    connect(m_destRadioButtons, SIGNAL(buttonClicked(int)), this, SLOT(onDestSelectionChanged()));
    connect(m_destList, SIGNAL(currentRowChanged(int)), this, SLOT(onDestSelectionChanged()));
    connect(m_destCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onDestSelectionChanged()));
    connect(m_destFilter, SIGNAL(textChanged(QString)), this, SLOT(onDestFilterTextChanged(QString)));
    connect(btns, SIGNAL(accepted()), this, SLOT(onAccept()));
    connect(btns, SIGNAL(rejected()), this, SLOT(reject()));

    rebuildBPicker();
    rebuildDestPicker();
    onDestChanged();
    setWindowTitle(tr("Binary Operation"));
    // resize(560, 560);
    updateSummary();

    Log_Debug3_T("");

    // QGroupBox* destGroup = new QGroupBox(tr("Destination"), this);
    // QVBoxLayout* destLay = new QVBoxLayout(destGroup);
    //
    // m_destOverwriteA = new QRadioButton(tr("Overwrite A"), destGroup);
    // m_destOverwriteB = new QRadioButton(tr("Overwrite B"), destGroup);
    // m_destCreateNew  = new QRadioButton(tr("Create new"),  destGroup);
    //
    // m_newNameEdit = new QLineEdit(destGroup);
    // m_newNameEdit->setEnabled(false);
    //
    // destLay->addWidget(m_destOverwriteA);
    // destLay->addWidget(m_destOverwriteB);
    //
    // QHBoxLayout* nameRow = new QHBoxLayout();
    // nameRow->addWidget(m_destCreateNew);
    // nameRow->addWidget(new QLabel(tr("Name:"), destGroup));
    // nameRow->addWidget(m_newNameEdit, 1);
    // QWidget* nameRowW = new QWidget(destGroup);
    // nameRowW->setLayout(nameRow);
    // destLay->addWidget(nameRowW);
    //
    // auto* destButtons = new QButtonGroup(destGroup);
    // destButtons->setExclusive(true);
    // destButtons->addButton(m_destOverwriteA);
    // destButtons->addButton(m_destOverwriteB);
    // destButtons->addButton(m_destCreateNew);
    //
    // connect(m_destOverwriteA, SIGNAL(toggled(bool)), this, SLOT(onDestChanged()));
    // connect(m_destOverwriteB, SIGNAL(toggled(bool)), this, SLOT(onDestChanged()));
    // connect(m_destCreateNew, SIGNAL(toggled(bool)), this, SLOT(onDestChanged()));
    //
    // m_summary = new QLabel(tr(""), this);
    // m_remember = new QCheckBox(tr("Remember B and destination"), this);
    //
    // m_buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, this);
    // connect(m_buttons, SIGNAL(accepted()), this, SLOT(onAccept()));
    // connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));
    //
    // root->addWidget(opsRowW);
    // root->addWidget(aRowW);
    // root->addWidget(bPicker);
    // root->addWidget(destGroup);
    // root->addWidget(m_summary);
    // root->addWidget(m_remember);
    // root->addWidget(m_buttons);
    //
    // setWindowTitle(tr("Binary Operation"));
    // resize(560, 560);
    // Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::rebuildBPicker() {
    Log_Debug3_H("");
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
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::selectBInUi(int indexB) {
    Log_Debug3_H("");
    if (m_names.isEmpty()) {
        m_indexB = 0;
        Log_Debug3_T("");
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
    Log_Debug3_T("m_indexB =" << m_indexB);
}


int mdn::gui::BinaryOperationDialog::currentBFromUi() const {
    Log_Debug3_H("");
    if (m_bRadioGroup->isVisible()) {
        int id = m_bRadioButtons->checkedId();
        Log_Debug3_T("");
        return id;
    } else {
        if (m_bList->isVisible()) {
        Log_Debug3_T("");
            return m_bList->currentRow();
        } else {
        Log_Debug3_T("");
            return m_bCombo->currentIndex();
        }
    }
}


void mdn::gui::BinaryOperationDialog::rebuildDestPicker() {
    Log_Debug3_H("");

    // Build friendly label list with (A)/(B) markers
    QStringList labeled = m_names;
    if (m_indexA >= 0 && m_indexA < labeled.size()) {
        labeled[m_indexA] = labeled[m_indexA] + "  (A)";
    }
    if (m_indexB >= 0 && m_indexB < labeled.size() && m_indexB != m_indexA) {
        labeled[m_indexB] = labeled[m_indexB] + "  (B)";
    }

    const int n = labeled.size();
    const bool small = n > 0 && n <= 10;
    const bool medium = n > 10 && n <= 25;

    if (small) {
        m_destRadioGroup->setVisible(true);
        m_destFilter->setVisible(false);
        m_destList->setVisible(false);
        m_destCombo->setVisible(false);

        QLayoutItem* child;
        while ((child = m_destRadioGroup->layout()->takeAt(0)) != nullptr) {
            if (child->widget()) child->widget()->deleteLater();
            delete child;
        }
        const int oldCount = m_destRadioButtons->buttons().size();
        for (QAbstractButton* b : m_destRadioButtons->buttons()) b->deleteLater();
        if (oldCount > 0) m_destRadioButtons->setExclusive(false);
        m_destRadioButtons->setExclusive(true);

        for (int i = 0; i < n; ++i) {
            QRadioButton* r = new QRadioButton(labeled[i], m_destRadioGroup);
            m_destRadioButtons->addButton(r, i);
            m_destRadioGroup->layout()->addWidget(r);
        }
    } else {
        m_destRadioGroup->setVisible(false);
        if (medium) {
            m_destFilter->setVisible(true);
            m_destList->setVisible(true);
            m_destCombo->setVisible(false);

            m_destList->clear();
            m_destList->addItems(labeled);
            if (n > 0) {
                m_destList->setCurrentRow(0);
            }
        } else {
            m_destFilter->setVisible(false);
            m_destList->setVisible(false);
            m_destCombo->setVisible(true);

            m_destCombo->clear();
            m_destCombo->addItems(labeled);
            if (n > 0) {
                m_destCombo->setCurrentIndex(0);
            }
        }
    }

    selectDestInUi(m_indexDest);
    Log_Debug3_T("");
}


void mdn::gui::BinaryOperationDialog::selectDestInUi(int index) {
    Log_Debug3_H("");
    const int n = m_names.size();
    if (n <= 0) {
        m_indexDest = 0;
        Log_Debug3_T("");
        return;
    }
    int idx = index;
    if (idx < 0) {
        idx = 0;
    } else {
        if (idx >= n) {
            idx = n - 1;
        }
    }
    if (m_destRadioGroup->isVisible()) {
        QAbstractButton* b = m_destRadioButtons->button(idx);
        if (b) {
            b->setChecked(true);
        }
    } else {
        if (m_destList->isVisible()) {
            m_destList->setCurrentRow(idx);
        } else {
            m_destCombo->setCurrentIndex(idx);
        }
    }
    m_indexDest = idx;
    Log_Debug3_T("m_indexDest=" << m_indexDest);
}


int mdn::gui::BinaryOperationDialog::currentDestFromUi() const {
    Log_Debug3_H("");
    if (m_destRadioGroup->isVisible()) {
        int id = m_destRadioButtons->checkedId();
        Log_Debug3_T("");
        return id;
    } else {
        if (m_destList->isVisible()) {
            Log_Debug3_T("");
            return m_destList->currentRow();
        } else {
            Log_Debug3_T("");
            return m_destCombo->currentIndex();
        }
    }
}


void mdn::gui::BinaryOperationDialog::updateSummary() {
    Log_Debug3_H("");
    QString a = m_names.value(m_indexA);
    QString b = m_names.value(m_indexB);
    QString sym = opSymbol(m_op);

    QString destStr;
    if (m_dest == DestinationMode::CreateNew) {
        destStr = QStringLiteral("Create \"%1\"").arg(m_newNameEdit->text());
    } else {
        QString dn = m_names.value(m_indexDest);
        if (m_indexDest == m_indexA) dn += "  (A)";
        if (m_indexDest == m_indexB) dn += "  (B)";
        destStr = QStringLiteral("Overwrite %1").arg(dn);
    }

    QString s = QStringLiteral("%1  %2  %3  →  %4").arg(a, sym, b, destStr);
    m_summary->setText(s);
    Log_Debug3_T("");
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