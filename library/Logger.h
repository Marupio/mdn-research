#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <mutex>

namespace mdn {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    void setEnabled(bool enable) { enabled = enable; }
    void setLevel(LogLevel level) { minLevel = level; }

    void log(LogLevel level, const std::string& msg) {
        // TODO - add more capability to output function name, file name, line number of caller
        if (!enabled || level < minLevel) return;
        std::lock_guard<std::mutex> lock(logMutex);
        std::cerr << "[" << levelToString(level) << "] " << msg << std::endl;
    }

    void debug(const std::string& msg) { log(LogLevel::Debug, msg); }
    void info(const std::string& msg) { log(LogLevel::Info, msg); }
    void warn(const std::string& msg) { log(LogLevel::Warning, msg); }
    void error(const std::string& msg) { log(LogLevel::Error, msg); }

private:
    Logger() = default;
    bool enabled = true;
    LogLevel minLevel = LogLevel::Info;
    std::mutex logMutex;

    std::string levelToString(LogLevel level) const {
        switch (level) {
            case LogLevel::Debug: return "Debug";
            case LogLevel::Info: return "Info";
            case LogLevel::Warning: return "Warning";
            case LogLevel::Error: return "Error";
            default: return "Unknown";
        }
    }
};

} // namespace mdn
