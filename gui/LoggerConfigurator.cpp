#include "LoggerConfigurator.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QStringList>
#include <QVersionNumber>

#include <filesystem>
#include <optional>
#include <sstream>

#include "../library/Tools.hpp"

// If you use your Qt->Logger bridge, keep installing it in main, not here.

using mdn::cli::LoggerConfigurator;

namespace mdn::cli {

LoggerConfigurator::LoggerConfigurator(QString appDescription)
    : m_optLogLevel(
          QStringList{QStringLiteral("l"), QStringLiteral("log-level")},
          QStringLiteral("Set log verbosity. One of: %1").arg(allowedLevelsList()),
          QStringLiteral("level")),
      m_optLogFile(
          QStringList{QStringLiteral("f"), QStringLiteral("log-file")},
          QStringLiteral("Write logs to the given file path (creates/overwrites)."),
          QStringLiteral("path")),
      m_optNoLogFile(
          QStringList{QStringLiteral("no-log-file")},
          QStringLiteral("Do not write logs to a file (overrides any defaults).")),
      m_optLogIndentation(
          QStringList{QStringLiteral("check-indents")},
          QStringLiteral("Monitor debug logging for missing indentation calls")),
      m_optJsonInput(
          QStringList{QStringLiteral("i"), QStringLiteral("input")},
          QStringLiteral("Path to JSON settings file."),
          QStringLiteral("path")),
      m_appDescription(std::move(appDescription)),
      m_userLogFileChoice(0)
{
    // Help + version always available
    m_parser.addHelpOption();
    m_parser.addVersionOption();
    if (!m_appDescription.isEmpty()) {
        m_parser.setApplicationDescription(m_appDescription);
    }
}


void LoggerConfigurator::addStandardOptions() {
    m_parser.addOption(m_optLogLevel);
    m_parser.addOption(m_optLogFile);
    m_parser.addOption(m_optNoLogFile);
    m_parser.addOption(m_optLogIndentation);
    m_parser.addOption(m_optJsonInput);
}


void LoggerConfigurator::addCustomOption(const QCommandLineOption& opt) {
    m_parser.addOption(opt);
}


bool LoggerConfigurator::process(QCoreApplication& app) {
    // Set defaults before reading CLI/JSON, matching current behaviour.
    applyDefaults();

    // Parse CLI
    m_parser.process(app);

    // Keep JSON path (if provided) for later
    if (m_parser.isSet(m_optJsonInput)) {
        m_jsonPath = m_parser.value(m_optJsonInput);
    }

    // Apply CLI -> Logger
    if (!applyCliToLogger()) {
        return false;
    }

    // Apply JSON -> Logger (only if provided)
    if (!applyJsonIfProvided()) {
        // We log a warning inside; continue running (non-fatal).
    }

    return true;
}

// ---------------- private helpers ----------------

void LoggerConfigurator::applyDefaults() {
    mdn::Logger& sirTalksAlot = mdn::Logger::instance();
    sirTalksAlot.setLevel(mdn::LogLevel::Info);
    // sirTalksAlot.setOutputToFile();
}


bool LoggerConfigurator::applyCliToLogger() {
    mdn::Logger& sirTalksAlot = mdn::Logger::instance();

    // --log-level
    if (m_parser.isSet(m_optLogLevel)) {
        const auto levelStr = m_parser.value(m_optLogLevel);
        if (auto lvl = parseLogLevel(levelStr)) {
            sirTalksAlot.setLevel(*lvl);
        } else {
            qCritical().noquote()
                << "Invalid log level:" << levelStr
                << "\nAllowed:" << allowedLevelsList();
            return false;
        }
    }

    // --no-log-file / --log-file
    if (m_parser.isSet(m_optNoLogFile)) {
        // Do nothing: the default setOutputToFile() above is intentionally overridden by "do nothing" here
        // so no file logging is enabled if caller uses --no-log-file.
        // sirTalksAlot.disableFileOutput(); // assuming your Logger has this; if not, remove and rely on "do nothing"
        m_userLogFileChoice = -1;
    } else if (m_parser.isSet(m_optLogFile)) {
        const auto p = m_parser.value(m_optLogFile);
        sirTalksAlot.setOutputToFile(std::filesystem::path{p.toStdString()});
    } else {
        sirTalksAlot.setOutputToFile();
    }

    // --check-indents
    if (m_parser.isSet(m_optLogIndentation)) {
        sirTalksAlot.enableIndentChecking();
    }

    return true;
}


bool LoggerConfigurator::applyJsonIfProvided() {
    if (!m_jsonPath.has_value()) return true;

    QJsonObject root;
    if (!loadJsonObjectFromFile(*m_jsonPath, root)) {
        Log_Warn("Ignoring settings file due to errors: " << m_jsonPath->toStdString());
        return false;
    }
    if (!applyLoggerSettingsFromJson(root)) {
        Log_Warn("Ignoring settings file due to errors: invalid 'Level' specified.");
        return false;
    }
    return true;
}


std::optional<mdn::LogLevel> LoggerConfigurator::parseLogLevel(QString s) {
    s = s.trimmed().toLower();
    s.remove(' ');

    if (s == "debug4" || s == "d4" || s == "4") return mdn::LogLevel::Debug4;
    if (s == "debug3" || s == "d3" || s == "3") return mdn::LogLevel::Debug3;
    if (s == "debug2" || s == "d2" || s == "2") return mdn::LogLevel::Debug2;
    if (s == "debug"  || s == "d"  || s == "1") return mdn::LogLevel::Debug;
    if (s == "info"   || s == "i"  || s == "0") return mdn::LogLevel::Info;
    if (s == "warning"|| s == "w")              return mdn::LogLevel::Warning;
    if (s == "error"  || s == "e")              return mdn::LogLevel::Error;
    return std::nullopt;
}


QString LoggerConfigurator::allowedLevelsList() {
    return QStringLiteral("Debug4 | Debug3 | Debug2 | Debug | Info | Warning | Error");
}


bool LoggerConfigurator::loadJsonObjectFromFile(const QString& path, QJsonObject& out) {
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


bool LoggerConfigurator::applyLoggerSettingsFromJson(const QJsonObject& root) {
    if (!root.contains("Logger")) {
        return false;
    }

    const QJsonObject logger = root.value("Logger").toObject();

    mdn::Logger& sirTalksAlot = mdn::Logger::instance();
    if (logger.contains("Level")) {
        const QString levelStr = logger.value("Level").toString();
        if (auto lvl = parseLogLevel(levelStr)) {
            sirTalksAlot.setLevel(*lvl);
        } else {
            qCritical().noquote()
                << "Invalid log level:" << levelStr
                << "\nAllowed:" << allowedLevelsList();
            return false;
        }
        Log_Info("Logger.Level set to " << levelStr.toStdString());
    }

    if (logger.contains("CheckIndents")) {
        bool checkIndents = logger.value("CheckIndents").toBool(false);
        if (checkIndents) {
            sirTalksAlot.enableIndentChecking();
            Log_Info("Indent-checking enabled");
        }
    }

    if (logger.contains("Output")) {
        const QString outPath = logger.value("Output").toString();
        if (!outPath.isEmpty()) {
            const std::filesystem::path p(outPath.toStdString());
            sirTalksAlot.setOutputToFile(p);
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
                if (!s.isEmpty()) includes.push_back(s.toStdString());
            }
        }

        if (filter.contains("Exclude")) {
            const QJsonArray arr = filter.value("Exclude").toArray();
            for (const QJsonValue& v : arr) {
                const QString s = v.toString();
                if (!s.isEmpty()) excludes.push_back(s.toStdString());
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
                if (i > 0) oss << ", ";
                oss << includes[i];
            }
            oss << "]";
            Log_Info(oss.str());
            std::string filterStr = mdn::Tools::vectorToString(includes, ", ", false);
            Log_Info("Applying Logger filter includes: " << filterStr);
            sirTalksAlot.setIncludes(includes);
        } else if (!excludes.empty()) {
            std::string filterStr = mdn::Tools::vectorToString(excludes, ", ", false);
            Log_Info("Applying Logger filter excludes: " << filterStr);
            sirTalksAlot.setExcludes(excludes);
        }
    }

    return true;
}

} // namespace mdn::cli
