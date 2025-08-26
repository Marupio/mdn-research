#pragma once

#include <QDialog>
#include <QStringList>

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
    enum class Operation {
        Add,
        Subtract,
        Multiply,
        Divide
    };

    enum class DestinationMode {
        OverwriteA,
        OverwriteB,
        CreateNew
    };

    struct Plan {
        Operation op;
        int indexA;
        int indexB;
        DestinationMode dest;
        QString newName;
        bool rememberChoices;
    };

public:
    explicit BinaryOperationDialog(QWidget* parent = nullptr);

    void setTabNames(const QStringList& names);
    void setActiveIndex(int indexA);
    void setInitialOperation(Operation op);
    void setRememberedB(int indexB);
    void setRememberedDestination(DestinationMode mode);

    Plan plan() const;

private slots:
    void onOpChanged();
    void onBSelectionChanged();
    void onDestChanged();
    void onFilterTextChanged(const QString& text);
    void onAccept();

private:
    void buildUi();
    void rebuildBPicker();
    void selectBInUi(int indexB);
    int currentBFromUi() const;
    void updateSummary();
    QString opSymbol(Operation op) const;
    QString opName(Operation op) const;

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

    QRadioButton* m_destOverwriteA{nullptr};
    QRadioButton* m_destOverwriteB{nullptr};
    QRadioButton* m_destCreateNew{nullptr};
    QLineEdit* m_newNameEdit{nullptr};

    QLabel* m_summary{nullptr};
    QDialogButtonBox* m_buttons{nullptr};
    QCheckBox* m_remember{nullptr};
};

} // end namespace gui
} // end namespace mdn
