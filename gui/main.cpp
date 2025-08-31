#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QStringList>

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../library/Logger.hpp"
#include "../library/Tools.hpp"
#include "MainWindow.hpp"
#include "QtLoggingBridge.hpp"


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


// Read file into QJsonObject, returns true on success
static bool loadJsonObjectFromFile(const QString& path, QJsonObject& out)
{
    QFile f(path);

    if (!f.open(QIODevice::ReadOnly)) {
        mdn::Logger::instance().warn(std::string("Failed to open settings file: ") + path.toStdString());
        return false;
    }

    const QByteArray bytes = f.readAll();
    f.close();

    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);

    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        mdn::Logger::instance().warn(std::string("Invalid JSON settings: ") + path.toStdString());
        return false;
    }

    out = doc.object();
    return true;
}


// Apply Logger.* from JSON object
static bool applyLoggerSettingsFromJson(const QJsonObject& root)
{
    if (!root.contains("Logger")) {
        return false;
    }

    const QJsonObject logger = root.value("Logger").toObject();

    mdn::Logger& sirTalksALot = mdn::Logger::instance();
    if (logger.contains("Level")) {
        const QString levelStr = logger.value("Level").toString();
        if (auto lvl = parseLogLevel(levelStr)) {
            sirTalksALot.setLevel(*lvl);
        } else {
            qCritical().noquote()
                << "Invalid log level:" << levelStr
                << "\nAllowed:" << allowedLevelsList();
            return false;
        }
        Log_Info("Logger.Level set to " << levelStr.toStdString());
    }

    if (logger.contains("Output")) {
        const QString outPath = logger.value("Output").toString();
        if (!outPath.isEmpty()) {
            const std::filesystem::path p(outPath.toStdString());
            sirTalksALot.setOutputToFile(p);
            Log_Info("Logger.Output set to " << p.string());
        }
    }

    if (logger.contains("Filter")) {
        const QJsonObject filter = logger.value("Filter").toObject();

        std::vector<std::string> includes;
        std::vector<std::string> excludes;

        if (filter.contains("Include")) {
            const QJsonArray arr = filter.value("Include").toArray();
            for (const QJsonValue& v : arr) {
                const QString s = v.toString();
                if (!s.isEmpty()) {
                    includes.push_back(s.toStdString());
                }
            }
        }

        if (filter.contains("Exclude")) {
            const QJsonArray arr = filter.value("Exclude").toArray();
            for (const QJsonValue& v : arr) {
                const QString s = v.toString();
                if (!s.isEmpty()) {
                    excludes.push_back(s.toStdString());
                }
            }
        }

        if (!includes.empty() && !excludes.empty()) {
            Log_Warn("Logger.Filter has both Include and Exclude; ignoring Exclude");
            excludes.clear();
        }

        if (!includes.empty()) {
            std::ostringstream oss;
            oss << "Logger.Filter.Include = [";
            for (size_t i = 0; i < includes.size(); ++i) {
                if (i > 0) {
                    oss << ", ";
                }
                oss << includes[i];
            }
            oss << "]";
            Log_Info(oss.str());
            std::string filterStr = mdn::Tools::vectorToString(includes, ", ", false);
            Log_Info("Applying Logger filter includes: " << filterStr);
            sirTalksALot.setIncludes(includes);
        } else if (!excludes.empty()) {
            std::string filterStr = mdn::Tools::vectorToString(excludes, ", ", false);
            Log_Info("Applying Logger filter excludes: " << filterStr);
            sirTalksALot.setExcludes(excludes);
        }
    }
    return true;
}


int main(int argc, char *argv[]) {
    mdn_installQtMessageHandler();
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

    // Optional: --no-log-file to explicitly keep logging off file even if code default changes later
    QCommandLineOption optLogIndentation(
        QStringList{QStringLiteral("check-indents")},
        QStringLiteral("Monitor debug logging for missing indentation calls")
    );

    QCommandLineOption inputOpt(
        QStringList() << "i" << "input",
        QStringLiteral("Path to JSON settings file."),
        QStringLiteral("path")
    );

    parser.addOption(optLogLevel);
    parser.addOption(optLogFile);
    parser.addOption(optLogIndentation);
    parser.addOption(optNoLogFile);
    parser.addOption(inputOpt);
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
        // uses Logger's default path
        sirTalksALot.setOutputToFile();
    }

    // Enable indentation checking, if necessary
    if (parser.isSet(optLogIndentation)) {
        sirTalksALot.enableIndentChecking();
    }

    // Apply settings if provided
    if (parser.isSet(inputOpt)) {
        const QString cfgPath = parser.value(inputOpt);
        QJsonObject root;
        const bool ok = loadJsonObjectFromFile(cfgPath, root);
        if (ok) {
            if (!applyLoggerSettingsFromJson(root)) {
                Log_Warn("Ignoring settings file due to errors: invalid 'Level' specified.");
            }
        } else {
            Log_Warn("Ignoring settings file due to errors: " << cfgPath.toStdString());
        }
    }

    mdn::gui::MainWindow window;
    window.show();
    return app.exec();
}

