#include <QApplication>
#include "QtLoggingBridge.hpp"
#include "LoggerConfigurator.hpp"   // <-- new

#include "MainWindow.hpp"

int main(int argc, char* argv[]) {
    mdn_installQtMessageHandler();

    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("mdn_gui");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    // Build the common CLI+JSON logger setup
    mdn::cli::LoggerConfigurator logCfg("MDN GUI");
    logCfg.addStandardOptions();

    // If you have app-specific options, you can add them here:
    // QCommandLineOption myOpt({"x","example"}, "Example flag");
    // logCfg.addCustomOption(myOpt);

    if (!logCfg.process(app)) {
        return EXIT_FAILURE;
    }

    // If you need to read app-specific options after process():
    // auto& parser = logCfg.parser();
    // if (parser.isSet(myOpt)) { ... }

    mdn::gui::MainWindow w;
    w.show();
    return app.exec();
}
