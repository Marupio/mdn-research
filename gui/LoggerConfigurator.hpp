#pragma once

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QJsonObject>

#include <optional>
#include <string>
#include <vector>

#include <mdn/Logger.hpp>

namespace mdn::cli {

class LoggerConfigurator {
public:
    // Construct with your application name/description (for --help text).
    explicit LoggerConfigurator(QString appDescription = {});

    // Add the standard options (-l/--log-level, -f/--log-file, --no-log-file, --check-indents, -i/--input).
    // You may call addCustomOption(...) before process() to add your own options too.
    void addStandardOptions();

    // Optional: let callers register extra options that should appear in --help.
    // (Add them before calling process().)
    void addCustomOption(const QCommandLineOption& opt);

    // Parse args and configure mdn::Logger accordingly.
    // Returns false if there was an error (e.g., invalid log level or bad JSON).
    bool process(QCoreApplication& app);

    // After process(): access to the parser (e.g., to read your custom options).
    const QCommandLineParser& parser() const { return m_parser; }

    // Convenience getters (valid after process()).
    std::optional<QString> jsonPath() const { return m_jsonPath; }

private:
    // ---- helpers ----
    static std::optional<mdn::LogLevel> parseLogLevel(QString s);
    static QString allowedLevelsList();

    static bool loadJsonObjectFromFile(const QString& path, QJsonObject& out);
    static bool applyLoggerSettingsFromJson(const QJsonObject& root);

    void applyDefaults();                 // default logger setup before CLI
    bool applyCliToLogger();              // uses m_parser
    bool applyJsonIfProvided();           // uses m_jsonPath

    QCommandLineParser m_parser;
    QCommandLineOption m_optLogLevel;
    QCommandLineOption m_optLogFile;
    QCommandLineOption m_optNoLogFile;
    QCommandLineOption m_optLogIndentation;
    QCommandLineOption m_optJsonInput;
    QCommandLineOption m_optInfo;
    // QCommandLineOption m_optDebug;

    std::optional<QString> m_jsonPath;
    QString m_appDescription;

    // Did the user make a choice about the log file:
    //  1   User specified a log file
    //  0   User made no choice
    // -1   User specified 'no log file'
    int m_userLogFileChoice;
};

} // namespace mdn::cli
