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
    void setOutputToFileLegacy(std::filesystem::path debugFile="");
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

    std::string levelToString(LogLevel level) const {
        // All aligned to improve log readability
        switch (level) {
            case LogLevel::Debug4: return  " Debug4";
            case LogLevel::Debug3: return  " Debug3";
            case LogLevel::Debug2: return  " Debug2";
            case LogLevel::Debug: return   "  Debug";
            case LogLevel::Info: return    "   Info";
            case LogLevel::Warning: return "Warning";
            case LogLevel::Error: return   "  Error";
            default: return                "Unknown";
        }
    }

    // Return the indent, in integer form
    int getIndent() const { return m_indent; }

    // Return the indent, in string form
    const std::string& indent() const { return m_indentStr; }

    // Increase the indent by two spaces
    void increaseIndent() {
        m_indent += 2;
        m_indentStr += "  ";
    }

    // Reduce the indent by two spaces
    void decreaseIndent() {
        m_indent -= 2;
        m_indentStr.clear();
        for(int i=0; i < m_indent; ++i) {
            m_indentStr += "  ";
        }
    }


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
    int m_indent;
    std::string m_indentStr;

    // Return the default path for the log file - not a member variable due to library linkage
    //  issues
    static const std::filesystem::path& defaultPath();
    static std::filesystem::path m_debugLog;
};


//  Logger macros:
//    Messaging macros, for adding __FILE__ and __LINE__ to the output message, syntax:
//      Log_Debug2("message and " << variable << " stream operators okay");
//    Conditional macros, for protecting resource-intensive messages, syntax:
//      if (Log_Showing_Debug2) {
//          // Resource-intensive operations to produce the message
//          Log_Debug2("output message");
//      }


// Assertion macros
// Internal use - the FileRef, intended to be wrapped by a string ctor, produces a nicely
//  formatted prefix to each log entry:
//      [Mdn2dBase.cpp:598,setValue]
#define InternalLoggerFileRef \
        "[" + Tools::removePath(__FILE__) + ":" + std::to_string(__LINE__) + "," + \
        __func__ + "] "

#define Internal_AssertStart(expression, messageIfFailed) \
    if (!(expression)) { \
        std::ostringstream oss; \
        oss << InternalLoggerFileRef << messageIfFailed << std::endl; \
        FailedAssertion err = FailedAssertion(oss.str().c_str()); \
        Logger& loginst = Logger::instance(); \
        loginst.error(err.what());

// The library is independent of GUI frameworks, but here exists a macro allowing a QMessageBox,
//  allowing GUI code to call this macro in a place where QMessageBox is defined
#define Internal_AssertQCrit QMessageBox::critical(nullptr, "Failed Assert", err.what());
#define Internal_AssertEnd throw err; }

#define Assert(expression, messageIfFailed) \
    Internal_AssertStart(expression, messageIfFailed) \
    Internal_AssertEnd
#define AssertQ(expression, messageIfFailed) \
    Internal_AssertStart(expression, messageIfFailed) \
    Internal_AssertQCrit \
    Internal_AssertEnd


#ifdef MDN_LOGS

    #define DBAssert(expression, messageIfFailed) Assert(expression, messageIfFailed)
    #define DBAssertQ(expression, messageIfFailed) AssertQ(expression, messageIfFailed)

    // Internal use - this macro brings together the final logging code
    #define InternalLoggerAssembleFunction(FILE_REF, message, level) { \
        std::ostringstream oss; \
        std::string fileRef( \
            FILE_REF \
        ); \
        Logger& loginst = Logger::instance(); \
        oss << loginst.indent() << fileRef << message; \
        loginst.level(oss.str());

    #define InternalLoggerEchoToQ QMessageBox("Logger", oss.str().c_str()); }

    // Internal use - As above, but with indentation:
    //      [        |Mdn2dBase.cpp:598,setValue]
    #define InternalIdentedLoggerFileRef \
            "[" + std::to_string(Logger::instance().getIndent()) + "|" \
            + Tools::removePath(__FILE__) + ":" + std::to_string(__LINE__) + "," + \
            __func__ + "] "

    // Internal use - appends m_name, an object's member variable, to the end of the file ref:
    //      [        |Mdn2dBase.cpp:598,setValue] Mdn2d_2_Copy_1:
    //  Obviously only useful if the class has member variable:
    //      std::string m_name;
    #define InternalLoggerNamedFileRef InternalIdentedLoggerFileRef + "(" + m_name + ") "

    // Standard log message:
    //  No header / footer indent / unindent
    //  File ref with object name
    #define InternalLoggerNamed(message, level) \
        InternalLoggerAssembleFunction( InternalLoggerNamedFileRef, message, level ) \
        }

    // Standard log message:
    //  No header / footer indent / unindent
    //  File ref with object name
    //  Echo to QMessageBox
    #define InternalLoggerNamedQ(message, level) \
        InternalLoggerAssembleFunction( InternalLoggerNamedFileRef, message, level ) \
        InternalLoggerEchoToQ

    // Standard log message:
    //  No header / footer indent / unindent
    //  File ref with no object name
    #define InternalLoggerAnonymous(message, level) \
        InternalLoggerAssembleFunction( InternalIdentedLoggerFileRef, message, level ) \
        }

    // Standard log message:
    //  No header / footer indent / unindent
    //  File ref with no object name
    //  Echo to QMessageBox
    #define InternalLoggerAnonymousQ(message, level) \
        InternalLoggerAssembleFunction( InternalIdentedLoggerFileRef, message, level ) \
        InternalLoggerEchoToQ


    // Internal use - Wrapper for producing a header message that increases the indentation level
    #define InternalLoggerHeaderWrapper(LOGGER_MACRO, message, level) { \
        std::string msgStr; \
        { \
            std::ostringstream oss; \
            oss << message; \
            msgStr = oss.str(); \
        } \
        if (msgStr.empty()) { \
            Logger::instance().increaseIndent(); \
            LOGGER_MACRO(msgStr, level); \
        } else { \
            LOGGER_MACRO("", level); \
            Logger::instance().increaseIndent(); \
            LOGGER_MACRO(msgStr, level); \
        } \
    }

    // Standard log message:
    //  Header with an increase in indentation
    //  File ref with no object name
    #define InternalLoggerAnonymousHeader(message, level) \
        InternalLoggerHeaderWrapper(InternalLoggerAnonymous, message, level)

    // Standard log message:
    //  Header with an increase in indentation
    //  File ref with object name
    #define InternalLoggerNamedHeader(message, level) \
        InternalLoggerHeaderWrapper(InternalLoggerNamed, message, level)


    // Internal use - Wrapper for producing a footer message that decreases the indentation level
    #define InternalLoggerFooterWrapper(LOGGER_MACRO, message, level) { \
        LOGGER_MACRO(message, level); \
        Logger::instance().decreaseIndent(); \
    }

    // Standard log message:
    //  Footer with a decrease in indentation
    //  File ref with no object name
    #define InternalLoggerAnonymousFooter(message, level) \
        InternalLoggerFooterWrapper(InternalLoggerAnonymous, message, level)

    // Standard log message:
    //  Footer with a decrease in indentation
    //  File ref with object name
    #define InternalLoggerNamedFooter(message, level) \
        InternalLoggerFooterWrapper(InternalLoggerNamed, message, level)


    // Macro intended to be used within a conditional, returns true if level would be displayed
    #define InternalLoggerQuery(level) (Logger::instance().isShowing(level))

    // *** Dev facing macros

    // Queries
    #define Log_Showing_Debug4 InternalLoggerQuery(LogLevel::Debug4)
    #define Log_Showing_Debug3 InternalLoggerQuery(LogLevel::Debug3)
    #define Log_Showing_Debug2 InternalLoggerQuery(LogLevel::Debug2)
    #define Log_Showing_Debug InternalLoggerQuery(LogLevel::Debug)
    #define Log_Showing_Info InternalLoggerQuery(LogLevel::Info)
    #define Log_Showing_Warn InternalLoggerQuery(LogLevel::Warn)
    #define Log_Showing_Error InternalLoggerQuery(LogLevel::Error)

    // Anonymous, no changes in indentation
    #define Log_Debug4(message) InternalLoggerAnonymous(message, debug4)
    #define Log_Debug3(message) InternalLoggerAnonymous(message, debug3)
    #define Log_Debug2(message) InternalLoggerAnonymous(message, debug2)
    #define Log_Debug(message) InternalLoggerAnonymous(message, debug)
    #define Log_Info(message) InternalLoggerAnonymous(message, info)
    #define Log_Warn(message) InternalLoggerAnonymous(message, warn)
    #define Log_Error(message) InternalLoggerAnonymous(message, error)

    // Anonymous, no changes in indentation, echo to QMessageBox
    #define Log_Debug4Q(message) InternalLoggerAnonymousQ(message, debug4)
    #define Log_Debug3Q(message) InternalLoggerAnonymousQ(message, debug3)
    #define Log_Debug2Q(message) InternalLoggerAnonymousQ(message, debug2)
    #define Log_DebugQ(message) InternalLoggerAnonymousQ(message, debug)
    #define Log_InfoQ(message) InternalLoggerAnonymousQ(message, info)
    #define Log_WarnQ(message) InternalLoggerAnonymousQ(message, warn)
    #define Log_ErrorQ(message) InternalLoggerAnonymousQ(message, error)

    // Anonymous, headers - increase in indentation
    #define Log_Debug4_H(message) InternalLoggerAnonymousHeader(message, debug4)
    #define Log_Debug3_H(message) InternalLoggerAnonymousHeader(message, debug3)
    #define Log_Debug2_H(message) InternalLoggerAnonymousHeader(message, debug2)
    #define Log_Debug_H(message) InternalLoggerAnonymousHeader(message, debug)

    // Anonymous, footers - decrease in indentation
    #define Log_Debug4_T(message) InternalLoggerAnonymousFooter(message, debug4)
    #define Log_Debug3_T(message) InternalLoggerAnonymousFooter(message, debug3)
    #define Log_Debug2_T(message) InternalLoggerAnonymousFooter(message, debug2)
    #define Log_Debug_T(message) InternalLoggerAnonymousFooter(message, debug)

    // Named, no changes in indentation
    #define Log_N_Debug4(message) InternalLoggerNamed(message, debug4)
    #define Log_N_Debug3(message) InternalLoggerNamed(message, debug3)
    #define Log_N_Debug2(message) InternalLoggerNamed(message, debug2)
    #define Log_N_Debug(message) InternalLoggerNamed(message, debug)
    #define Log_N_Info(message) InternalLoggerNamed(message, info)
    #define Log_N_Warn(message) InternalLoggerNamed(message, warn)
    #define Log_N_Error(message) InternalLoggerNamed(message, error)

    // Named, no changes in indentation, echo to QMessageBox
    #define Log_N_Debug4Q(message) InternalLoggerNamedQ(message, debug4)
    #define Log_N_Debug3Q(message) InternalLoggerNamedQ(message, debug3)
    #define Log_N_Debug2Q(message) InternalLoggerNamedQ(message, debug2)
    #define Log_N_DebugQ(message) InternalLoggerNamedQ(message, debug)
    #define Log_N_InfoQ(message) InternalLoggerNamedQ(message, info)
    #define Log_N_WarnQ(message) InternalLoggerNamedQ(message, warn)
    #define Log_N_ErrorQ(message) InternalLoggerNamedQ(message, error)

    // Named, headers - increase in indentation
    #define Log_N_Debug4_H(message) InternalLoggerNamedHeader(message, debug4)
    #define Log_N_Debug3_H(message) InternalLoggerNamedHeader(message, debug3)
    #define Log_N_Debug2_H(message) InternalLoggerNamedHeader(message, debug2)
    #define Log_N_Debug_H(message) InternalLoggerNamedHeader(message, debug)

    // Named, footers - decrease in indentation
    #define Log_N_Debug4_T(message) InternalLoggerNamedFooter(message, debug4)
    #define Log_N_Debug3_T(message) InternalLoggerNamedFooter(message, debug3)
    #define Log_N_Debug2_T(message) InternalLoggerNamedFooter(message, debug2)
    #define Log_N_Debug_T(message) InternalLoggerNamedFooter(message, debug)

#else
    #define DBAssert(expression, messageIfFailed) do {} while (false);
    #define DBAssertQ(expression, messageIfFailed) do {} while (false);

    #define Log_Debug4(message)   do {} while (false);
    #define Log_Debug3(message)   do {} while (false);
    #define Log_Debug2(message)   do {} while (false);
    #define Log_Debug(message)    do {} while (false);
    #define Log_Info(message)     do {} while (false);
    #define Log_Warn(message)     do {} while (false);
    #define Log_Error(message)    do {} while (false);
    #define Log_Debug4Q(message)   do {} while (false);
    #define Log_Debug3Q(message)   do {} while (false);
    #define Log_Debug2Q(message)   do {} while (false);
    #define Log_DebugQ(message)    do {} while (false);
    #define Log_InfoQ(message)     do {} while (false);
    #define Log_WarnQ(message)     do {} while (false);
    #define Log_ErrorQ(message)    do {} while (false);
    #define Log_Debug4_H(message) do {} while (false);
    #define Log_Debug3_H(message) do {} while (false);
    #define Log_Debug2_H(message) do {} while (false);
    #define Log_Debug_H(message)  do {} while (false);
    #define Log_Debug4_T(message) do {} while (false);
    #define Log_Debug3_T(message) do {} while (false);
    #define Log_Debug2_T(message) do {} while (false);
    #define Log_Debug_T(message)  do {} while (false);
    #define Log_N_Debug4(message)   do {} while (false);
    #define Log_N_Debug3(message)   do {} while (false);
    #define Log_N_Debug2(message)   do {} while (false);
    #define Log_N_Debug(message)    do {} while (false);
    #define Log_N_Info(message)     do {} while (false);
    #define Log_N_Warn(message)     do {} while (false);
    #define Log_N_Error(message)    do {} while (false);
    #define Log_N_Debug4Q(message)   do {} while (false);
    #define Log_N_Debug3Q(message)   do {} while (false);
    #define Log_N_Debug2Q(message)   do {} while (false);
    #define Log_N_DebugQ(message)    do {} while (false);
    #define Log_N_InfoQ(message)     do {} while (false);
    #define Log_N_WarnQ(message)     do {} while (false);
    #define Log_N_ErrorQ(message)    do {} while (false);
    #define Log_N_Debug4_H(message) do {} while (false);
    #define Log_N_Debug3_H(message) do {} while (false);
    #define Log_N_Debug2_H(message) do {} while (false);
    #define Log_N_Debug_H(message)  do {} while (false);
    #define Log_N_Debug4_T(message) do {} while (false);
    #define Log_N_Debug3_T(message) do {} while (false);
    #define Log_N_Debug2_T(message) do {} while (false);
    #define Log_N_Debug_T(message)  do {} while (false);
    #define Log_Showing_Debug4    false
    #define Log_Showing_Debug3    false
    #define Log_Showing_Debug2    false
    #define Log_Showing_Debug     false
    #define Log_Showing_Info      false
    #define Log_Showing_Warn      false
    #define Log_Showing_Error     false
#endif



} // namespace mdn
