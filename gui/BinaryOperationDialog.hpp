#pragma once

#include <QDialog>
#include <QStringList>

#include "EnumDestinationMode.hpp"
#include "EnumOperation.hpp"
#include "OperationPlan.hpp"
#include <mdn/Logger.hpp>

class QAbstractButton;
class QButtonGroup;
class QComboBox;
class QDialogButtonBox;
class QGroupBox;
class QLineEdit;
class QListWidget;
class QLabel;
class QRadioButton;
class QStackedLayout;
class QCheckBox;
class QWidget;

namespace mdn {
namespace gui {

class BinaryOperationDialog : public QDialog {
    Q_OBJECT

public:
    explicit BinaryOperationDialog(QWidget* parent = nullptr);

    void setTabNames(const QStringList& names);
    void setActiveIndex(int indexA);
    void setInitialOperation(Operation op);
    void setRememberedB(int indexB);
    void setRememberedDestination(DestinationMode mode);

    OperationPlan plan() const;

private slots:
    void onOpChanged();
    void onBSelectionChanged();
    void onDestChanged();
    void onDestSelectionChanged();
    void onDestFilterTextChanged(const QString& text);
    void onBFilterTextChanged(const QString& text);
    void onAccept();

private:
    void buildUi();

    // Helpers for B picker
    void rebuildBPicker();
    void selectBInUi(int indexB);
    int currentBFromUi() const;

    // Helpers for dest picker
    void rebuildDestPicker();
    void selectDestInUi(int index);
    int currentDestFromUi() const;

    void updateSummary();
    QString opSymbol(Operation op) const;
    QString opName(Operation op) const;
    QString makeUnique(const QString& base, const QStringList& existing) const;

private:
    QStringList m_names;
    int m_indexA{0};
    int m_indexB{0};
    Operation m_op{Operation::Add};
    DestinationMode m_dest{DestinationMode::OverwriteA};

    QWidget* m_opsRow{nullptr};
    QButtonGroup* m_opsButtons{nullptr};

    QLabel* m_labelA{nullptr};

    QWidget* m_bPickerStack{nullptr};
    QGroupBox* m_bRadioGroup{nullptr};
    QButtonGroup* m_bRadioButtons{nullptr};
    QListWidget* m_bList{nullptr};
    QLineEdit* m_bFilter{nullptr};
    QComboBox* m_bCombo{nullptr};


    // QRadioButton* m_destOverwriteA{nullptr};
    // QRadioButton* m_destOverwriteB{nullptr};
    // QRadioButton* m_destCreateNew{nullptr};

    // Destination picker
    QWidget* m_destPickerStack{ nullptr };
    QGroupBox* m_destRadioGroup{ nullptr };
    QButtonGroup* m_destRadioButtons{ nullptr };
    QListWidget* m_destList{ nullptr };
    QLineEdit* m_destFilter{ nullptr };
    QComboBox* m_destCombo{ nullptr };

    // Destination radio roots
    QRadioButton* m_destOverwrite{ nullptr };
    QRadioButton* m_destCreateNew{ nullptr };

    int m_indexDest{ -1 };

    QLineEdit* m_newNameEdit{nullptr};

    QLabel* m_summary{nullptr};
    QCheckBox* m_remember{nullptr};
};

} // end namespace gui
} // end namespace mdn
