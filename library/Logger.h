#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <mutex>

#include "GlobalConfig.h"
#include "Tools.h"

namespace mdn {

enum class LogLevel {
    Debug4,
    Debug3,
    Debug2,
    Debug,
    Info,
    Warning,
    Error
};

class MDN_API Logger {
public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    void setEnabled(bool enable) { m_enabled = enable; }
    void setLevel(LogLevel level) { m_minLevel = level; }

    // Echo output messages to log file 'debugFile', leave blank for default log file location
    void setOutputToFile(std::filesystem::path debugFile="");

    LogLevel getLevel() { return m_minLevel; }
    bool isEnabled() { return m_enabled; }
    bool isShowing(LogLevel level) { return m_enabled && level >= m_minLevel; }

    void log(LogLevel level, const std::string& msg) {
        if (!m_enabled || level < m_minLevel) return;
        std::lock_guard<std::mutex> lock(m_logMutex);
        std::string levelStr = "[" + levelToString(level) + "] ";
        std::cerr << levelStr << msg << std::endl;
        if (m_ossPtr) {
            (*m_ossPtr) << levelStr << msg << std::endl;
        }
    }

    void debug4(const std::string& msg) { log(LogLevel::Debug4, msg); }
    void debug3(const std::string& msg) { log(LogLevel::Debug3, msg); }
    void debug2(const std::string& msg) { log(LogLevel::Debug2, msg); }
    void debug(const std::string& msg) { log(LogLevel::Debug, msg); }
    void info(const std::string& msg) { log(LogLevel::Info, msg); }
    void warn(const std::string& msg) { log(LogLevel::Warning, msg); }
    void error(const std::string& msg) { log(LogLevel::Error, msg); }

private:
    Logger() = default;
    ~Logger() {
        if (m_ossPtr) {
            m_ossPtr->close();
        }
    }
    bool m_enabled = true;
    LogLevel m_minLevel = LogLevel::Info;
    std::mutex m_logMutex;
    static std::ofstream* m_ossPtr;

    // Return the default path for the log file - not a member variable due to library linkage
    //  issues
    static const std::filesystem::path& defaultPath();
    static std::filesystem::path m_debugLog;

    std::string levelToString(LogLevel level) const {
        switch (level) {
            case LogLevel::Debug4: return "Debug4";
            case LogLevel::Debug3: return "Debug3";
            case LogLevel::Debug2: return "Debug2";
            case LogLevel::Debug: return "Debug";
            case LogLevel::Info: return "Info";
            case LogLevel::Warning: return "Warning";
            case LogLevel::Error: return "Error";
            default: return "Unknown";
        }
    }
};


//  Logger macros:
//    Messaging macros, for adding __FILE__ and __LINE__ to the output message, syntax:
//      Log_Debug2("message and " << variable << " stream operators okay");
//    Conditional macros, for protecting resource-intensive messages, syntax:
//      if (Log_Showing_Debug2) {
//          // Resource-intensive operations to produce the message
//          Log_Debug2("output message");
//      }

#ifdef MDN_DEBUG
    #define InternalLoggerMacro(message, level) { \
        std::ostringstream oss; \
        std::string fileRef( \
            "[" + Tools::removePath(__FILE__) + ":" + std::to_string(__LINE__) + "," + \
            __func__ + "] " \
        ); \
        oss << fileRef << message; \
        Logger::instance().level(oss.str()); \
    }
    #define InternalLoggerQuery(level) (Logger::instance().isShowing(level))

    #define Log_Debug4(message) InternalLoggerMacro(message, debug4)
    #define Log_Debug3(message) InternalLoggerMacro(message, debug3)
    #define Log_Debug2(message) InternalLoggerMacro(message, debug2)
    #define Log_Debug(message) InternalLoggerMacro(message, debug)
    #define Log_Info(message) InternalLoggerMacro(message, info)
    #define Log_Warn(message) InternalLoggerMacro(message, warn)
    #define Log_Error(message) InternalLoggerMacro(message, error)

    #define Log_Showing_Debug4 InternalLoggerQuery(LogLevel::Debug4)
    #define Log_Showing_Debug3 InternalLoggerQuery(LogLevel::Debug3)
    #define Log_Showing_Debug2 InternalLoggerQuery(LogLevel::Debug2)
    #define Log_Showing_Debug InternalLoggerQuery(LogLevel::Debug)
    #define Log_Showing_Info InternalLoggerQuery(LogLevel::Info)
    #define Log_Showing_Warn InternalLoggerQuery(LogLevel::Warn)
    #define Log_Showing_Error InternalLoggerQuery(LogLevel::Error)
#else
    #define Log_Debug4(message)   do {} while (false);
    #define Log_Debug3(message)   do {} while (false);
    #define Log_Debug2(message)   do {} while (false);
    #define Log_Debug(message)    do {} while (false);
    #define Log_Info(message)     do {} while (false);
    #define Log_Warn(message)     do {} while (false);
    #define Log_Error(message)    do {} while (false);
    #define Log_Showing_Debug4    false
    #define Log_Showing_Debug3    false
    #define Log_Showing_Debug2    false
    #define Log_Showing_Debug     false
    #define Log_Showing_Info      false
    #define Log_Showing_Warn      false
    #define Log_Showing_Error     false
#endif



} // namespace mdn
