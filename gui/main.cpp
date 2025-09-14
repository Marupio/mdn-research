#include <QApplication>
#include "QtLoggingBridge.hpp"
#include "LoggerConfigurator.hpp"
#include "WelcomeDialog.hpp"
#include "MainWindow.hpp"

int main(int argc, char* argv[]) {
    mdn_installQtMessageHandler();

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("mdn_gui");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    // Build the common CLI+JSON logger setup
    mdn::cli::LoggerConfigurator logCfg("MDN GUI");
    logCfg.addStandardOptions();
    if (!logCfg.process(app)) {
        return EXIT_FAILURE;
    }

    // Show launcher first
    WelcomeDialog launcher;
    const int rc = launcher.exec();
    const auto choice = launcher.choice();

    // If user cancelled/closed without choosing, treat as Exit.
    if (
        rc == QDialog::Rejected && choice != WelcomeDialog::Choice::NewProject
        && choice != WelcomeDialog::Choice::OpenProject
        && choice != WelcomeDialog::Choice::OpenRecent
    ) {
        return EXIT_SUCCESS;
    }



    mdn::gui::MainWindow w;
    w.show();
    return app.exec();
}
