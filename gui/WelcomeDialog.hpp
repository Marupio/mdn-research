#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QFrame>
#include <QPixmap>
#include <QStyle>
#include <QMessageBox>

#include "HelpDialog.hpp"

class WelcomeDialog : public QDialog {
    Q_OBJECT
public:
    enum class Choice { None, NewProject, OpenProject, Exit };

    static std::string ChoiceToString(Choice c) {
        switch (c) {
            case Choice::NewProject: return "NewProject";
            case Choice::OpenProject: return "OpenProject";
            case Choice::Exit: return "Exit";
            default: return "None";
        }
    }

    explicit WelcomeDialog(QWidget* parent = nullptr)
        : QDialog(parent), m_choice(Choice::None)
    {
        setWindowTitle(tr("Welcome to MDN"));
        setModal(true);
        // resize(640, 420);

        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(16, 16, 16, 16);
        root->setSpacing(16);

        // ========== Header (image + title/subtitle) ==========
        auto* header = new QWidget(this);
        auto* headerLay = new QHBoxLayout(header);
        headerLay->setContentsMargins(0, 0, 0, 0);
        headerLay->setSpacing(12);

        // Optional image (from resource). Put your image at :/images/welcome.png
        auto* pic = new QLabel(header);
        pic->setFixedSize(96, 96);
        QPixmap pm(":/images/welcome.png");
        if (!pm.isNull()) {
            pic->setPixmap(pm.scaled(pic->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            pic->hide(); // no image available
        }

        auto* titleBox = new QWidget(header);
        auto* ttl = new QLabel(tr("Multi-Dimensional Numbers (MDN)"), titleBox);
        auto* sub = new QLabel(tr("Research Toolkit"), titleBox);
        ttl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        sub->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        // Fonts: title big, subtitle slightly smaller
        QFont tf = ttl->font();
        tf.setPointSize(std::max(14, tf.pointSize() + 4));
        tf.setBold(true);
        ttl->setFont(tf);

        QFont sf = sub->font();
        sf.setPointSize(tf.pointSize() - 1);
        sf.setBold(true);
        sub->setFont(sf);
        ttl->setAlignment(Qt::AlignCenter);
        sub->setAlignment(Qt::AlignCenter);
        // sub->setStyleSheet("color: palette(mid);"); // subtle

        auto* titleLay = new QVBoxLayout(titleBox);
        titleLay->setContentsMargins(0, 0, 0, 0);
        titleLay->setSpacing(2);
        titleLay->addWidget(ttl);
        titleLay->addWidget(sub);

        // Beside layout:
        headerLay->addWidget(pic);
        headerLay->addWidget(titleBox, /*stretch*/1);
        // If you prefer the picture ABOVE the texts, swap the two lines above with:
        // auto* v = new QVBoxLayout(header);
        // v->addWidget(pic, 0, Qt::AlignHCenter);
        // v->addWidget(titleBox);
        // header->setLayout(v);

        root->addWidget(header);

        // ========== Content rows (two columns) ==========
        auto* rows = new QWidget(this);
        auto* rowsLay = new QHBoxLayout(rows);
        rowsLay->setContentsMargins(0, 0, 0, 0);
        rowsLay->setSpacing(16);

        // Left column: New / Open / Exit
        auto* left = new QWidget(rows);
        auto* leftLay = new QVBoxLayout(left);
        leftLay->setContentsMargins(0, 0, 0, 0);
        leftLay->setSpacing(8);

        auto mkBtn = [this, left](const QIcon& ic, const QString& text) {
            auto* b = new QPushButton(ic, text, left);
            b->setMinimumWidth(160);
            b->setIconSize(QSize(18, 18));
            return b;
        };

        auto icNew   = QIcon::fromTheme("document-new",   style()->standardIcon(QStyle::SP_FileDialogNewFolder));
        auto icOpen  = QIcon::fromTheme("document-open",  style()->standardIcon(QStyle::SP_DirOpenIcon));
        auto icExit  = QIcon::fromTheme("application-exit", style()->standardIcon(QStyle::SP_DialogCloseButton));

        auto* btnNew  = mkBtn(icNew,  tr("New"));
        auto* btnOpen = mkBtn(icOpen, tr("Open"));
        //auto* sepLine = new QFrame(left); sepLine->setFrameShape(QFrame::HLine); sepLine->setFrameShadow(QFrame::Sunken);
        auto* btnExit = mkBtn(icExit, tr("Exit"));

        leftLay->addSpacing(42);
        leftLay->addWidget(btnNew);
        leftLay->addWidget(btnOpen);
        // leftLay->addWidget(sepLine);
        leftLay->addWidget(btnExit);
        leftLay->addStretch(1);

        // Right column: Help card with rounded border + centred title
        auto* helpBox = new QGroupBox(tr("Help"), rows);
        auto* helpLay = new QVBoxLayout(helpBox);
        helpLay->setContentsMargins(12, 12, 12, 12);
        helpLay->setSpacing(8);

        // Style the box: rounded corners, centred title intersecting border
        helpBox->setStyleSheet(
            "QGroupBox {"
            "  border: 1px solid palette(mid);"
            "  border-radius: 10px;"
            "  margin-top: 14px;"          // leave room for the title
            "}"
            "QGroupBox::title {"
            "  subcontrol-origin: margin;"
            "  subcontrol-position: top center;"  // centre the title
            "  padding: 0 6px;"
            "}"
        );

        auto mkHelpBtn = [helpBox](const QString& text) {
            auto* b = new QPushButton(text, helpBox);
            b->setFlat(false);
            b->setMinimumWidth(160);
            return b;
        };

        auto* btnOverview = mkHelpBtn(tr("Overview"));
        auto* btnGuide    = mkHelpBtn(tr("Guide"));
        auto* btnAbout    = mkHelpBtn(tr("About"));
        auto* btnLicense  = mkHelpBtn(tr("License"));

        helpLay->addWidget(btnOverview);
        helpLay->addWidget(btnGuide);
        helpLay->addWidget(btnAbout);
        helpLay->addWidget(btnLicense);

        rowsLay->addWidget(left, /*stretch*/0);
        rowsLay->addWidget(helpBox, /*stretch*/1);

        root->addWidget(rows);

        // ========== Wiring ==========
        connect(btnNew,  &QPushButton::clicked, this, [this]{ m_choice = Choice::NewProject;  accept(); });
        connect(btnOpen, &QPushButton::clicked, this, [this]{ m_choice = Choice::OpenProject; accept(); });
        connect(btnExit, &QPushButton::clicked, this, [this]{ m_choice = Choice::Exit; reject(); });

        // Help buttons open modal help pages (dialog stays open after close)
        connect(btnOverview, &QPushButton::clicked, this, [this]{
            HelpDialog dlg("overview", this); dlg.exec();
        });
        connect(btnGuide, &QPushButton::clicked, this, [this]{
            HelpDialog dlg("guide", this);    dlg.exec();
        });
        connect(btnAbout, &QPushButton::clicked, this, [this]{
            HelpDialog dlg("about", this);    dlg.exec();
        });
        connect(btnLicense, &QPushButton::clicked, this, [this]{
            HelpDialog dlg("license", this);  dlg.exec();
        });
    }

    Choice choice() const noexcept { return m_choice; }

private:
    Choice m_choice;
};
