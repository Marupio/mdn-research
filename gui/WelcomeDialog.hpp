#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>

#include "HelpDialog.hpp"

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
    static std::string ChoiceToString(Choice c) {
        switch (c) {
            case Choice::NewProject: return "NewProject";
            case Choice::OpenProject: return "OpenProject";
            // case Choice::OpenRecent: return "OpenRecent";
            case Choice::Exit: return "Exit";
            default: return "None";
        }
    }

    explicit WelcomeDialog(QWidget* parent=nullptr)
      : QDialog(parent)
      , m_choice(Choice::None)
    {
        setWindowTitle(tr("Welcome to MDN"));
        setModal(true);

        auto* layout = new QVBoxLayout(this);

        auto* title = new QLabel(tr("Multi-Dimensional Number (MDN)"), this);
        auto* subTitle = new QLabel(tr("Research Toolkit"), this);
        title->setAlignment(Qt::AlignCenter);
        subTitle->setAlignment(Qt::AlignCenter);
        QFont f = title->font();
        f.setPointSize(18);
        title->setFont(f);
        subTitle->setFont(f);
        layout->addWidget(title);
        layout->addWidget(subTitle);

        auto* newBtn    = new QPushButton(tr("New Project"), this);
        // auto* recentBtn = new QPushButton(tr("Open Recent"), this);
        auto* openBtn   = new QPushButton(tr("Open Project"), this);
        auto* overviewBtn  = new QPushButton(tr("Overview"), this);
        auto* guideBtn  = new QPushButton(tr("Guide"), this);
        auto* aboutBtn  = new QPushButton(tr("About"), this);
        auto* licenseBtn  = new QPushButton(tr("License"), this);
        // auto* donateBtn = new QPushButton(tr("Donate"), this);
        auto* exitBtn   = new QPushButton(tr("Exit"), this);

        layout->addWidget(newBtn);
        // layout->addWidget(recentBtn);
        layout->addWidget(openBtn);
        layout->addWidget(overviewBtn);
        layout->addWidget(guideBtn);
        layout->addWidget(aboutBtn);
        layout->addWidget(licenseBtn);
        // layout->addWidget(donateBtn);
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
        connect(overviewBtn, &QPushButton::clicked, this, [this]{
            HelpDialog dlg("overview", this);
            dlg.exec();
        });
        connect(guideBtn, &QPushButton::clicked, this, [this]{
            HelpDialog dlg("guide", this);
            dlg.exec();
        });
        connect(aboutBtn, &QPushButton::clicked, this, [this]{
            HelpDialog dlg("about", this);
            dlg.exec();
        });
        connect(licenseBtn, &QPushButton::clicked, this, [this]{
            HelpDialog dlg("license", this);
            dlg.exec();
        });
        // connect(donateBtn, &QPushButton::clicked, this, []{
        //     QDesktopServices::openUrl(QUrl("https://example.org/donate"));
        // });

        // Slightly nicer default size
        resize(420, 360);
    }

    Choice choice() const noexcept { return m_choice; }

private:
    Choice m_choice;
};
