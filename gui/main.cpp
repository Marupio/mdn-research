#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QLabel>
#include <QStringList>

#include <optional>
#include <unordered_map>

#include "MainWindow.hpp"


static std::optional<mdn::LogLevel> parseLogLevel(QString s)
{
    s = s.trimmed().toLower();
    s.remove(' '); // allow "Debug 3" style

    // Accept long, short, and minimal aliases
    if (s == "debug4" || s == "d4" || s == "4") return mdn::LogLevel::Debug4;
    if (s == "debug3" || s == "d3" || s == "3") return mdn::LogLevel::Debug3;
    if (s == "debug2" || s == "d2" || s == "2") return mdn::LogLevel::Debug2;
    if (s == "debug"  || s == "d"  || s == "1") return mdn::LogLevel::Debug;
    if (s == "info"   || s == "i"  || s == "0") return mdn::LogLevel::Info;
    if (s == "warning"|| s == "w")              return mdn::LogLevel::Warning;
    if (s == "error"  || s == "e")              return mdn::LogLevel::Error;
    return std::nullopt;
}

static QString allowedLevelsList()
{
    return QStringLiteral("Debug4 | Debug3 | Debug2 | Debug | Info | Warning | Error");
}


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Defaults (can be overridden by CLI)
    mdn::Logger& sirTalksALot = mdn::Logger::instance();
    sirTalksALot.setLevel(mdn::LogLevel::Info);
    // Not setting file by default; CLI can do it. If you want current default behaviour:
    // sirTalksALot.setOutputToFile(); // empty path means your default behaviour

    // --- CLI parsing
    QCoreApplication::setApplicationName("mdn_gui");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);

    QCommandLineParser parser;
    parser.setApplicationDescription("MDN GUI");
    parser.addHelpOption();
    parser.addVersionOption();

    // -l / --log-level
    QCommandLineOption optLogLevel(
        {QStringLiteral("l"), QStringLiteral("log-level")},
        QStringLiteral("Set log verbosity. One of: %1").arg(allowedLevelsList()),
        QStringLiteral("level")
    );

    // -f / --log-file
    QCommandLineOption optLogFile(
        {QStringLiteral("f"), QStringLiteral("log-file")},
        QStringLiteral("Write logs to the given file path (creates/overwrites)."),
        QStringLiteral("path")
    );

    // Optional: --no-log-file to explicitly keep logging off file even if code default changes later
    QCommandLineOption optNoLogFile(
        QStringList{QStringLiteral("no-log-file")},
        QStringLiteral("Do not write logs to a file (overrides any defaults).")
    );

    parser.addOption(optLogLevel);
    parser.addOption(optLogFile);
    parser.addOption(optNoLogFile);
    parser.process(app);

    // Apply log level
    if (parser.isSet(optLogLevel)) {
        const auto levelStr = parser.value(optLogLevel);
        if (auto lvl = parseLogLevel(levelStr)) {
            sirTalksALot.setLevel(*lvl);
        } else {
            qCritical().noquote()
                << "Invalid log level:" << levelStr
                << "\nAllowed:" << allowedLevelsList();
            return EXIT_FAILURE;
        }
    }

    // Apply log file routing
    if (parser.isSet(optNoLogFile)) {
        // Intentionally do nothing (ensures we don't enable file output below even if a default exists)
    } else if (parser.isSet(optLogFile)) {
        const auto p = parser.value(optLogFile);
        sirTalksALot.setOutputToFile(std::filesystem::path{p.toStdString()});
    } else {
        // Keep whatever default you want here. If your current default is “log to file” with default path:
        // sirTalksALot.setOutputToFile(); // uses your Logger's default path
    }

    MainWindow window;
    window.show();
    return app.exec();
}

