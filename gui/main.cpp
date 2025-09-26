#include <QApplication>

#include "../library/Logger.hpp"
#include "../library/Mdn2dConfig.hpp"

#include "mdn_config.h"
#include "QtLoggingBridge.hpp"
#include "LoggerConfigurator.hpp"
#include "MainWindow.hpp"
#include "ProjectPropertiesDialog.hpp"
#include "WelcomeDialog.hpp"

using namespace mdn;
using namespace mdn::gui;

// One iteration of WelcomeDialog --> RunApplication
bool runLauncher(QApplication& app) {
    // Show launcher first
    Log_Debug2_H("");
    WelcomeDialog launcher;
    const int rc = launcher.exec();
    const auto choice = launcher.choice();
    Log_Debug2("choice = " << WelcomeDialog::ChoiceToString(choice));

    // If user cancelled/closed without choosing, treat as Exit.
    if (
        rc == QDialog::Rejected && choice != WelcomeDialog::Choice::NewProject
        && choice != WelcomeDialog::Choice::OpenProject
        // && choice != WelcomeDialog::Choice::OpenRecent
    ) {
        Log_Debug2("");
        return false;
    }
    Log_Debug2("");

    // Decide how to start MainWindow
    // mdn::gui::MainWindow mainApp;
    // Log_Debug2("");

    switch (choice) {
        case WelcomeDialog::Choice::NewProject: {
            // Blank project - open ProjectPropertiesDialog
            Log_Debug2("");
            Mdn2dConfig cfg = Mdn2dConfig::static_defaultConfig();
            ProjectPropertiesDialog dlg(nullptr, nullptr);
            Log_Debug2("");
            dlg.setInitial(
                QString::fromStdString("untitled"),
                QString::fromStdString(""),
                cfg
            );
            if (dlg.exec() != QDialog::Accepted) {
                Log_Debug2_T("User rejected change");
                return true;
            }

            cfg = dlg.chosenConfig();
            Log_Debug2("cfg=" << cfg);
            mdn::gui::MainWindow mainApp(nullptr, &cfg);

            // false= do not ask for confirmation to close existing (dummy, empty) project
            mainApp.createNewProjectFromConfig(cfg, false);
            Log_Debug2("");
            mainApp.show();
            mainApp.centreView(0);
            bool result = (app.exec() == 0);
            Log_Debug2_T("result=" << result);
            return result;
        }

        case WelcomeDialog::Choice::OpenProject: {
            // Launch and immediately open the project picker
            mdn::gui::MainWindow mainApp;
            mainApp.show();
            // false= do not ask for confirmation to close existing (dummy, empty) project
            mainApp.openProject(false);
            // QTimer::singleShot(0, &mainApp, [&mainApp](){ mainApp.onOpenProject(); });
            mainApp.centreView(0);
            bool result = (app.exec() == 0);
            Log_Debug2_T("result=" << result);
            return result;
        }

        // case WelcomeDialog::Choice::OpenRecent: {
        //     // If you have an "open recent" entry point, trigger it here.
        //     // Placeholder pattern (adjust the slot/method name to your code):
        //     mainApp.show();
        //     QTimer::singleShot(0, &mainApp, [&mainApp](){
        //         // mainApp.onOpenRecent(); // TODO: call your actual recent-item flow
        //         mainApp.onOpenProject();  // temporary fallback
        //     });
        //     break;
        // }

        default: {
            // Safety net
            Log_Debug2_T("returning false");
            return false;
        }
    }
}


int main(int argc, char* argv[]) {
    mdn_installQtMessageHandler();

    QApplication app(argc, argv);

    // --- App identity for QSettings, file paths, etc. (set this EARLY) ---
    QCoreApplication::setOrganizationName("Gadensoft");
    // QCoreApplication::setOrganizationDomain("gadensoft.ca");
    QCoreApplication::setApplicationName("MDN Research");
    QCoreApplication::setApplicationVersion(QStringLiteral(MDN_TOOLS_VERSION_STR));

    // Build the common CLI+JSON logger setup
    mdn::cli::LoggerConfigurator logCfg("MDN GUI");
    logCfg.addStandardOptions();
    if (!logCfg.process(app)) {
        return EXIT_FAILURE;
    }

    bool dontStop = true;

    while (dontStop) {
        Log_Debug("");
        dontStop = runLauncher(app);
        Log_Debug("dontStop=" << dontStop);
    }

    return EXIT_SUCCESS;
}