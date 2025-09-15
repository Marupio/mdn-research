#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>

class WelcomeDialog : public QDialog {
    Q_OBJECT
public:
    enum class Choice {
        None,
        NewProject,
        OpenProject,
        // OpenRecent,
        Exit
    };

    explicit WelcomeDialog(QWidget* parent=nullptr)
      : QDialog(parent)
      , m_choice(Choice::None)
    {
        setWindowTitle(tr("Welcome to MDN"));
        setModal(true);

        auto* layout = new QVBoxLayout(this);

        auto* title = new QLabel(tr("MultiDimensional Number Research"), this);
        title->setAlignment(Qt::AlignCenter);
        QFont f = title->font();
        f.setPointSize(18);
        title->setFont(f);
        layout->addWidget(title);

        auto* newBtn    = new QPushButton(tr("New Project"), this);
        // auto* recentBtn = new QPushButton(tr("Open Recent"), this);
        auto* openBtn   = new QPushButton(tr("Open Project"), this);
        auto* aboutBtn  = new QPushButton(tr("About"), this);
        auto* donateBtn = new QPushButton(tr("Donate"), this);
        auto* exitBtn   = new QPushButton(tr("Exit"), this);

        layout->addWidget(newBtn);
        // layout->addWidget(recentBtn);
        layout->addWidget(openBtn);
        layout->addWidget(aboutBtn);
        layout->addWidget(donateBtn);
        layout->addWidget(exitBtn);

        // Closing choices (hand the decision back to main.cpp)
        connect(newBtn, &QPushButton::clicked, this, [this]{
            m_choice = Choice::NewProject;
            accept();
        });
        connect(openBtn, &QPushButton::clicked, this, [this]{
            m_choice = Choice::OpenProject;
            accept();
        });
        // connect(recentBtn, &QPushButton::clicked, this, [this]{
        //     m_choice = Choice::OpenRecent;
        //     accept();
        // });
        connect(exitBtn, &QPushButton::clicked, this, [this]{
            m_choice = Choice::Exit;
            reject(); // we’ll treat reject as a clean exit choice
        });

        // Non-closing helpers (keep dialog open)
        connect(aboutBtn, &QPushButton::clicked, this, [this]{
            QMessageBox::about(this, tr("About MDN"),
                               tr("<b>MDN GUI</b><br/>"
                                  "MultiDimensional Numbers research & tooling."));
        });
        connect(donateBtn, &QPushButton::clicked, this, []{
            QDesktopServices::openUrl(QUrl("https://example.org/donate"));
        });

        // Slightly nicer default size
        resize(420, 360);
    }

    Choice choice() const noexcept { return m_choice; }

private:
    Choice m_choice;
};
