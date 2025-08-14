----- Carryover.h -----
#pragma once

// Carryovers
//  Some carryovers are required, such as when a digit magnitude exceeds its base.  Other times a
//  carryover is optional - some MDNs have multiple valid states.  Switching between these states is
//  achieved using carryovers.  And some carryovers bring a number to an invalid state and should
//  not be performed.
//
//  In general, there are three types of carryovers:
//
//      * Invalid:    Valid MDN --> carryover --> Invalid MDN
//      * Optional:   Valid MDN --> carryover --> Valid MDN
//      * Required: Invalid MDN --> carryover --> Valid MDN
//
//  A  3    |   4
//     0 -3 | -10 -2
//
//  B  0    |   1
//     4 0  |  -6 1
//
//  C  0    |   1
//     4 3  |  -6 4
//
//  D  0    |   1
//     4 -3 |  -6 -2
//
//  E -2    |  -1
//     4  3 |  -6  4
//
//  F  2    |   3
//     4  3 |  -6  4
//
//  G -2    |  -1
//     4 -3 |  -6 -2
//
//    |  p  |  x  |  y  | comment
//    +-----+-----+-----+---------------------
// A  |  0  |  ?  |  ?  | not possible
//
// G  |  +  |  -  |  -  | required
// D  |  +  |  -  |  0  | optional
// E  |  +  |  +  |  -  | optional
// B  |  +  |  0  |  0  | invalid
// C  |  +  |  +  |  0  | invalid
// F  |  +  |  +  |  +  | invalid
//
// M  |  -  |  +  |  +  | required
// J  |  -  |  +  |  0  | optional
// K  |  -  |  -  |  +  | optional
// H  |  -  |  0  |  0  | invalid
// I  |  -  |  -  |  0  | invalid
// L  |  -  |  -  |  -  | invalid

// Interactions between polymorphic molecules
//
//  a  3    |   3      a'
//     0 -3 |   1 -3
//     2 -3 |  -8 -2
//
// In [a] above:
//  * the [2] is a polymorphic node, and
//  * the [0] is an invalid carryover
// However, in [a'] above:
//  * the [-8] is a polymorphic node (still), but
//  * the [1] is now a newly created polymorphic node
//
// Let's consider the [0] in [a] to be a dormant polymorphic node.  A polymorphic carryover can
// awaken a dormant polymorphic node.
//
//  b  3    |   3      b'
//     0 -3 |   1 -3
//     2 -3 |  -8 -2
//
//
// Further testing shows that a carryover can alter the neighbouring digit's carryover status, at
// any time.

#include <vector>
#include <string>

#include "Logger.h"

// Carryover
//  Type of carryover, depending on the root digit and the axial digits

namespace mdn {

enum class Carryover {
    Invalid,            // Valid MDN --> carryover --> Invalid MDN
    OptionalPositive,   // Valid MDN --> carryover --> Valid MDN, current value positive
    OptionalNegative,   // Valid MDN --> carryover --> Valid MDN, current value negative
    Required            // Invalid MDN --> carryover --> Valid MDN
};

const std::vector<std::string> CarryoverNames(
    {
        "Invalid",
        "OptionalPositive",
        "OptionalNegative",
        "Required"
    }
);

inline std::string CarryoverToName(Carryover carryover) {
    int fi = int(carryover);
    // #ifdef MDN_DEBUG
    //     if (fi < 0 || fi >= CarryoverNames.size())
    //         Logger::instance().error("Carryover out of range: " + std::to_string(fi));
    // #endif
    return CarryoverNames[fi];
}

inline Carryover NameToCarryover(const std::string& name) {
    for (int i = 0; i < CarryoverNames.size(); ++i) {
        if (CarryoverNames[i] == name) {
            return static_cast<Carryover>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid Carryover type: " << name << " expecting:" << std::endl;
    if (CarryoverNames.size()) {
        oss << CarryoverNames[0];
    }
    for (auto iter = CarryoverNames.cbegin() + 1; iter != CarryoverNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
----- Constants.h -----
#pragma once

#include <limits>

#include "Digit.h"

namespace mdn {

namespace constants {

constexpr int intMin = std::numeric_limits<int>::min();
constexpr int intMax = std::numeric_limits<int>::max();
constexpr Digit DigitMin = std::numeric_limits<Digit>::min();
constexpr Digit DigitMax = std::numeric_limits<Digit>::max();

constexpr float floatSmall = 1e-6;
constexpr double doubleSmall = 1e-12;

} // end namspace constants

} // end namespace mdn
----- Coord.h -----
#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <functional>
#include <tuple>

#include "GlobalConfig.h"

namespace mdn {

class MDN_API Coord {
public:
    constexpr Coord(int x = 0, int y = 0) : m_x(x), m_y(y) {}

    // Element accessors
    int x() const { return m_x; }
    int& x() { return m_x; }
    int y() const { return m_y; }
    int& y() { return m_y; }

    void set(int x, int y) {
        m_x = x;
        m_y = y;
    }

    // *** Functions that create a new Coord instance

        // Creates a new Coord, translated by the given (xIncr, yIncr)
        Coord translated(int xIncr, int yIncr) const {
            Coord ret(*this);
            ret.m_x += xIncr;
            ret.m_y += yIncr;
            return ret;
        }
        // Creates a new Coord, translated along x by the given xIncr
        Coord translatedX(int xIncr) const {
            Coord ret(*this);
            ret.m_x += xIncr;
            return ret;
        }
        // Creates a new Coord, translated along y by the given yIncr
        Coord translatedY(int yIncr) const {
            Coord ret(*this);
            ret.m_y += yIncr;
            return ret;
        }


    // *** Functions that modify *this

        // Move the Coord by the given (xIncr, yIncr)
        void translate(int xIncr, int yIncr) { m_x += xIncr; m_y += yIncr; }
        // Move the Coord along x by the given xIncr
        void translateX(int xIncr) { m_x += xIncr; }
        // Move the Coord along y by the given yIncr
        void translateY(int yIncr) { m_y += yIncr; }

    // Compound operators
    Coord& operator+=(int val) {
        m_x += val;
        m_y += val;
        return *this;
    }

    Coord& operator*=(int val) {
        m_x *= val;
        m_y *= val;
        return *this;
    }

    Coord& operator/=(int val) {
        m_x /= val;
        m_y /= val;
        return *this;
    }

    Coord& operator-=(int val) {
        m_x -= val;
        m_y -= val;
        return *this;
    }

    Coord& operator+=(const Coord& other) {
        m_x += other.m_x;
        m_y += other.m_y;
        return *this;
    }

    Coord& operator-=(const Coord& other) {
        m_x -= other.m_x;
        m_y -= other.m_y;
        return *this;
    }

    // Arithmetic operators
    Coord operator+(int val) const {
        return Coord(*this) += val;
    }

    Coord operator-(int val) const {
        return Coord(*this) -= val;
    }

    Coord operator*(int val) const {
        return Coord(*this) *= val;
    }

    Coord operator/(int val) const {
        return Coord(*this) /= val;
    }

    Coord operator+(const Coord& other) const {
        return Coord(*this) += other;
    }

    Coord operator-(const Coord& other) const {
        return Coord(*this) -= other;
    }

    bool operator==(const Coord& other) const {
        return m_x == other.m_x && m_y == other.m_y;
    }

    bool operator!=(const Coord& other) const {
        return !(*this == other);
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const Coord& c) {
        return os << "(" << c.m_x << ", " << c.m_y << ")";
    }

    friend std::istream& operator>>(std::istream& is, Coord& c) {
        char lparen, comma, rparen;
        return (is >> lparen >> c.m_x >> comma >> c.m_y >> rparen);
    }

    // Structured binding support
    friend constexpr int get(const Coord& c, std::integral_constant<std::size_t, 0>) noexcept {
        return c.m_x;
    }
    friend constexpr int get(const Coord& c, std::integral_constant<std::size_t, 1>) noexcept {
        return c.m_y;
    }


private:
    int m_x, m_y;
};

constexpr Coord COORD_ORIGIN = Coord{0, 0};

} // namespace mdn

namespace std {
    template <>
    struct hash<mdn::Coord> {
        std::size_t operator()(const mdn::Coord& c) const noexcept {
            std::size_t h1 = std::hash<int>{}(c.x());
            std::size_t h2 = std::hash<int>{}(c.y());
            return h1 ^ (h2 << 1);
        }
    };

    template <>
    struct tuple_size<mdn::Coord> : std::integral_constant<std::size_t, 2> {};

    template <std::size_t N>
    struct tuple_element<N, mdn::Coord> {
        using type = int;
    };

    template <std::size_t N>
    constexpr int get(const mdn::Coord& c) noexcept {
        static_assert(N < 2, "Index out of range for mdn::Coord");
        if constexpr (N == 0) return c.x();
        else return c.y();
    }
}

----- CoordSet.h -----
#pragma once

#include <unordered_set>

#include "Coord.h"

namespace mdn {

using CoordSet = std::unordered_set<Coord>;

} // end namespace mdn----- Digit.h -----
#pragma once

#include <cstdint>

namespace mdn {

using Digit = int8_t;

} // end namespace mdn
----- ErrorHandling.h -----
#pragma once

#include <string>

#include "GlobalConfig.h"

namespace mdn {

class MDN_API ErrorHandling {

public:

    static void PrintStackTrace();
    static void PrintStackTrace1();

    static std::string Demangle(const char* name);

};

} // end namespace mdn----- Fraxis.h -----
#pragma once

#include <vector>
#include <string>

#include "Logger.h"

// Fraxis - fraction axis
//  The digit axis along which the fractional part of a real number expands in an MDN

namespace mdn {

enum class Fraxis {
    Invalid,
    Default,
    X,
    Y
};

const std::vector<std::string> FraxisNames(
    {
        "Invalid",
        "Default",
        "X",
        "Y"
    }
);

inline std::string FraxisToName(Fraxis fraxis) {
    int fi = int(fraxis);
    // #ifdef MDN_DEBUG
    //     if (fi < 0 || fi >= FraxisNames.size())
    //         Logger::instance().error("Fraxis out of range: " + std::to_string(fi));
    // #endif
    return FraxisNames[fi];
}

inline Fraxis NameToFraxis(const std::string& name) {
    for (int i = 0; i < FraxisNames.size(); ++i) {
        if (FraxisNames[i] == name) {
            return static_cast<Fraxis>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid Fraxis type: " << name << " expecting:" << std::endl;
    if (FraxisNames.size()) {
        oss << FraxisNames[0];
    }
    for (auto iter = FraxisNames.cbegin() + 1; iter != FraxisNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
----- GlobalConfig.h -----
#pragma once

#ifdef _WIN32
  #ifdef mdn_EXPORTS
    #define MDN_API __declspec(dllexport)
  #else
    #define MDN_API __declspec(dllimport)
  #endif
#else
  #define MDN_API
#endif


constexpr const char* past_last_slash(const char* path) {
    const char* last = path;
    while (*path) {
        if (*path == '/' || *path == '\\') last = path + 1;
        ++path;
    }
    return last;
}

#define SHORT_FILE (past_last_slash(__FILE__))

// #define SHORT_FILE ({constexpr cstr sf__ {past_last_slash(__FILE__)}; sf__;})
// extern const char* PathToPrettyName(const char*);
// static const char* TranslationUnitName(const char* Path)
// {
// static const char* Name=PathToPrettyName(Path);
// return Name;
// }
// #define FILENAME__ TranslationUnitName(__FILE__)----- Logger.h -----
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
    if (!expression) { \
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

#ifdef MDN_DEBUG

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
        loginst.level(oss.str()); \
    }

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
    #define InternalLoggerNamedFileRef InternalIdentedLoggerFileRef + m_name + ": "

    // Standard log message:
    //  No header / footer indent / unindent
    //  File ref with object name
    #define InternalLoggerNamed(message, level) \
        InternalLoggerAssembleFunction( InternalLoggerNamedFileRef, message, level )


    // Standard log message:
    //  No header / footer indent / unindent
    //  File ref with no object name
    #define InternalLoggerAnonymous(message, level) \
        InternalLoggerAssembleFunction( InternalIdentedLoggerFileRef, message, level )


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
----- Mdn2d.h -----
#pragma once

#include "GlobalConfig.h"
#include "Mdn2dRules.h"

namespace mdn {

// Represents a 2D Multi-Dimensional Number (MDN).
class MDN_API Mdn2d : public Mdn2dRules {

protected:

public:

    // *** Constructors

        // Construct null
        Mdn2d(std::string nameIn="");

        // Construct from a configuration
        Mdn2d(Mdn2dConfig config, std::string nameIn="");


        // *** Rule of five

            // Copy constructor
            Mdn2d(const Mdn2d& other, std::string nameIn="");

            // Assignment operator
            Mdn2d& operator=(const Mdn2d& other);

            // Move operator
            Mdn2d(Mdn2d&& other, std::string nameIn="") noexcept;

            // Move assignment operator
            Mdn2d& operator=(Mdn2d&& other) noexcept;

            // Virtual destructor
            virtual ~Mdn2d() {}


    // *** Member Functions

        // *** Full Mdn2d mathematical operations

            // Addition: *this + rhs = ans, overwrites ans
            void plus(const Mdn2d& rhs, Mdn2d& ans) const;
            protected: CoordSet locked_plus(const Mdn2d& rhs, Mdn2d& ans) const; public:

            // Subtraction: *this - rhs = ans, overwrites ans
            void minus(const Mdn2d& rhs, Mdn2d& ans) const;
            protected: CoordSet locked_minus(const Mdn2d& rhs, Mdn2d& ans) const; public:

            // Multiplication: *this x rhs = ans, overwrites ans
            void multiply(const Mdn2d& rhs, Mdn2d& ans) const;
            protected: CoordSet locked_multiply(const Mdn2d& rhs, Mdn2d& ans) const; public:

            // Division: *this / rhs = ans, overwrites ans
            void divide(const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis=Fraxis::Default) const;
            protected: CoordSet locked_divide(
                const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis=Fraxis::Default
            ) const; public:
            protected: CoordSet locked_divideX(const Mdn2d& rhs, Mdn2d& ans) const; public:
            protected: CoordSet locked_divideY(const Mdn2d& rhs, Mdn2d& ans) const; public:


        // *** Addition / subtraction

            // Add the given number at xy, breaking into integer and fraxis operations
            void add(const Coord& xy, float realNum, Fraxis fraxis=Fraxis::Default);
            void add(const Coord& xy, double realNum, Fraxis fraxis=Fraxis::Default);
            protected: CoordSet locked_add(
                const Coord& xy, double realNum, Fraxis fraxis=Fraxis::Default
            ); public:

            // Subtract the given number at xy, breaking into integer and fraxis operations
            void subtract(const Coord& xy, float realNum, Fraxis fraxis=Fraxis::Default);
            void subtract(const Coord& xy, double realNum, Fraxis fraxis=Fraxis::Default);

            // Addition component: integer part, at xy with symmetric carryover
            void add(const Coord& xy, Digit value, Fraxis unused=Fraxis::Default);
            protected: CoordSet locked_add(const Coord& xy, Digit value); public:
            void add(const Coord& xy, int value, Fraxis unused=Fraxis::Default);
            protected: CoordSet locked_add(const Coord& xy, int value); public:
            void add(const Coord& xy, long value, Fraxis unused=Fraxis::Default);
            protected: CoordSet locked_add(const Coord& xy, long value); public:
            void add(const Coord& xy, long long value, Fraxis unused=Fraxis::Default);
            protected: CoordSet locked_add(const Coord& xy, long long value); public:

            // Subtraction component: integer part, at xy with symmetric carryover
            void subtract(const Coord& xy, Digit value, Fraxis unused=Fraxis::Default);
            void subtract(const Coord& xy, int value, Fraxis unused=Fraxis::Default);
            void subtract(const Coord& xy, long value, Fraxis unused=Fraxis::Default);
            void subtract(const Coord& xy, long long value, Fraxis unused=Fraxis::Default);

            // Addition component: fractional part, at xy with assymmetric cascade
            void addFraxis(const Coord& xy, float fraction, Fraxis fraxis=Fraxis::Default);
            void addFraxis(const Coord& xy, double fraction, Fraxis fraxis=Fraxis::Default);
            protected: CoordSet locked_addFraxis(
                const Coord& xy, double fraction, Fraxis fraxis
            ); public:

            // Subtract a fractional value cascading along the fraxis
            void subtractFraxis(const Coord& xy, float fraction, Fraxis fraxis=Fraxis::Default);
            void subtractFraxis(const Coord& xy, double fraction, Fraxis fraxis=Fraxis::Default);


        // *** Multiplication / divide

            // Multiply the full Mdn2d by an integer
            void multiply(Digit value);
            void multiply(int value);
            void multiply(long value);
            void multiply(long long value);
            protected: CoordSet locked_multiply(int value); public:
            protected: CoordSet locked_multiply(long value); public:
            protected: CoordSet locked_multiply(long long value); public:


    // *** Member Operators

        // Equality comparison
        //  The rules layer brings carryovers, allowing us to find equivalence between different
        //  states of polymorphism.  But for now, equivalence only works with a default sign
        //  convention (Mdn2dConfig)
        bool operator==(const Mdn2d& rhs) const;

        // Inequality comparison.
        bool operator!=(const Mdn2d& rhs) const;

        // Assignment addition, *this += rhs
        Mdn2d& operator+=(const Mdn2d& rhs);
        protected:
            // I'm different from unlocked: I return the changed set, but the standard += return is
            //  just *this
            CoordSet locked_plusEquals(const Mdn2d& rhs);
        public:

        // Assignment subtraction, *this -= rhs
        Mdn2d& operator-=(const Mdn2d& rhs);
        protected:
            // I'm different from unlocked: I return the changed set, but the standard -= return is
            //  just *this
            CoordSet locked_minusEquals(const Mdn2d& rhs);
        public:

        // Assignment multiplication, *this *= rhs
        Mdn2d& operator*=(const Mdn2d& rhs);
        protected:
            // I'm different from unlocked: I return the changed set, but the standard -= return is
            //  just *this
            CoordSet locked_timesEquals(const Mdn2d& rhs);
        public:

        // Placeholder for MDN division.
        Mdn2d& operator/=(const Mdn2d& rhs);

        // Scalar multiplication.
        Mdn2d& operator*=(int scalar);

        // Scalar multiplication.
        Mdn2d& operator*=(long scalar);

        // Scalar multiplication.
        Mdn2d& operator*=(long long scalar);


protected:

        // *** Internal functions
        // All these functions require the mutex to be locked first before calling

            // Apply default to fraxis as required
            void internal_checkFraxis(Fraxis& fraxis) const;

            // Execute the fraxis propagation algorithm
            //  dX, dY, c - constants to guide propagation:
            //      x Direction: -1, 0, -1
            //      y Direction: 0, -1, 1
            CoordSet internal_fraxis(const Coord& xy, double f, int dX, int dY, int c);
            CoordSet internal_fraxisCascade(const Coord& xy, Digit d, int c);

            // // plusEquals variant: *this += rhs x scalar, used in mdn x mdn algorithm
            // Mdn2d& internal_plusEquals(const Mdn2d& rhs, int scalar);

            // Creates a copy of *this and performs a multiply and shift, used in mdn x mdn
            //  return = (*this x value).shift(xy)
            Mdn2d internal_copyMultiplyAndShift(int value, const Coord& shiftXY) const;

};

// Arithmetic binary operators
inline Mdn2d operator+(Mdn2d lhs, const Mdn2d& rhs) { return lhs += rhs; }
inline Mdn2d operator-(Mdn2d lhs, const Mdn2d& rhs) { return lhs -= rhs; }
inline Mdn2d operator*(Mdn2d lhs, const Mdn2d& rhs) { return lhs *= rhs; }
inline Mdn2d operator/(Mdn2d lhs, const Mdn2d& rhs) { return lhs /= rhs; }

inline Mdn2d operator*(Mdn2d lhs, int scalar) { return lhs *= scalar; }

} // end namespace mdn
----- Mdn2dBase.h -----
#pragma once

#include <map>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "CoordSet.h"
#include "GlobalConfig.h"
#include "Mdn2dConfig.h"
#include "Mdn2dConfigImpact.h"
#include "MdnObserver.h"
#include "PrecisionStatus.h"
#include "Rect.h"

namespace mdn {

class Mdn2d;
class Project;

// Digit layer of 2d multi dimensional numbers, establishes:
//  * 2-dimensional digit representation
//  * sparse storage and addressing
//  * bounds metadata
class MDN_API Mdn2dBase {

    // *** Friends
    friend class Project;


protected:

    // Static name generation

    // Thread safety at static layer
    static std::shared_mutex m_static_mutex;

    // Used to generate the next defaulted name
    static int m_nextNameSeed;

    // Creates a new Mdn2d name, acquires static lock first
    static std::string static_generateNextName();

    // Creates a new Mdn2d name, assumes lock already acquired
    static std::string locked_generateNextName();

    // Create a 'copy' name from given nameIn (e.g. nameIn_copy0), acquires static lock first
    static std::string static_generateCopyName(const std::string& nameIn);

    // Create a 'copy' name from given nameIn (e.g. nameIn_copy0), assumes lock already acquired
    static std::string locked_generateCopyName(const std::string& nameIn);


    // *** Local variables

    // Configuration settings for this Mdn2dBase
    Mdn2dConfig m_config;

    // Thread safety
    mutable std::shared_mutex m_mutex;


    // *** Data & addressing

    // Name of this number
    std::string m_name;

    // Sparse coordinate-to-digit mapping
    std::unordered_map<Coord, Digit> m_raw;

    // Addressing
    mutable std::map<int, CoordSet> m_xIndex;
    mutable std::map<int, CoordSet> m_yIndex;

    // Full index
    mutable CoordSet m_index;

    // Observers
    mutable std::unordered_map<int, MdnObserver*> m_observers;


    // *** Metadata

        // Bounding box for non-zero digits
        mutable Rect m_bounds;

        // When true, increment m_event once operation is complete
        bool m_modified;

        // Event number for tracking derived, demand-driven data
        long long m_event;


public:

    // *** Typedefs

    using WritableLock = std::unique_lock<std::shared_mutex>;
    using ReadOnlyLock = std::shared_lock<std::shared_mutex>;


    // *** Static functions


    // Create a fully-realised Mdn2d instance, accessible from downstream layers
    static Mdn2d NewInstance(
        Mdn2dConfig config=Mdn2dConfig::static_defaultConfig(),
        std::string nameIn=""
    );
    static Mdn2d Duplicate(const Mdn2d& other, std::string nameIn="");


    // *** Constructors

        // Construct from a configuration, or default
        Mdn2dBase(std::string nameIn="");

        Mdn2dBase(Mdn2dConfig config, std::string nameIn="");


        // *** Rule of five

            // Copy constructor
            Mdn2dBase(const Mdn2dBase& other, std::string nameIn="");

            // Assignment operator
            Mdn2dBase& operator=(const Mdn2dBase& other);

            // Move operator
            Mdn2dBase(Mdn2dBase&& other, std::string nameIn="") noexcept;

            // Move assignment operator
            Mdn2dBase& operator=(Mdn2dBase&& other) noexcept;

            // Virtual destructor
            virtual ~Mdn2dBase();


    // *** Member Functions

        // *** Config

            // Return the Mdn2dConfig for this number
            const Mdn2dConfig& getConfig() const;
            protected: const Mdn2dConfig& locked_getConfig() const; public:

            // Assess the impact of changing the config to the supplied newConfig
            Mdn2dConfigImpact assessConfigChange(const Mdn2dConfig& newConfig);
            protected: Mdn2dConfigImpact locked_assessConfigChange(const Mdn2dConfig& newConfig); public:

            // Change the config - can lead to any of the Mdn2dConfigImpact effects
            void setConfig(Mdn2dConfig& newConfig);
            // Locked version *copies* config - everyone has their own copy
            protected: virtual void locked_setConfig(Mdn2dConfig newConfig); public:

        // *** Observers

            // Register a new observer
            void registerObserver(MdnObserver* obs) const;
            protected: void locked_registerObserver(MdnObserver* obs) const; public:

            // Unregister the owner (does not delete observer)
            void unregisterObserver(MdnObserver* obs) const;
            protected: void locked_unregisterObserver(MdnObserver* obs) const; public:


        // *** Identity

            // Return name
            const std::string& getName() const;
            protected: const std::string& locked_getName() const; public:

            // Set this number's 'name', deferring to framework for approval
            void setName(const std::string& nameIn);
            protected: void locked_setName(const std::string& nameIn); public:


        // *** Getters

            // Retrieves the value at coordinate (x, y), or 0 if not present.
            Digit getValue(const Coord& xy) const;
            protected: Digit locked_getValue(const Coord& xy) const; public:

            // Write out a continuous range for a row, from xStart to xEnd, inclusive.
            // out.size() becomes (x1 - x0 + 1). Returns false only on internal error.
            bool getRowRange
            (
                int y,
                int xStart,
                int xEnd,
                std::vector<mdn::Digit>& out
            ) const;
        protected:
            bool locked_getRowRange
            (
                int y,
                int xStart,
                int xEnd,
                std::vector<mdn::Digit>& out
            ) const;
        public:

            // Write a contiguous row back in one call. Interprets 0 as "setToZero".
            void setRowRange(int y, int xStart, const std::vector<mdn::Digit>& row);
            protected:
                void locked_setRowRange(int y, int xStart, const std::vector<mdn::Digit>& row);
            public:

            // Assembles the row at the given y index value, spanning the x bounds of full MDN
            std::vector<Digit> getRow(int y) const;
            protected: std::vector<Digit> locked_getRow(int y) const; public:

            // Assembles the row at the given y index value, spanning the x bounds of full MDN
            void getRow(int y, std::vector<Digit>& digits) const;
            protected: void locked_getRow(int y, std::vector<Digit>& digits) const; public:

            // Assembles the col at the given x index value, spanning the y bounds of full MDN
            std::vector<Digit> getCol(int x) const;
            protected: std::vector<Digit> locked_getCol(int x) const; public:

            // Assembles the col at the given x index value, spanning the y bounds of full MDN
            void getCol(int x, std::vector<Digit>& digits) const;
            protected: void locked_getCol(int x, std::vector<Digit>& digits) const; public:


        // *** Setters

            // Clears all digits in the MDN.
            void clear();
            protected: void locked_clear(); public:

            // Set the value at xy to zero
            //  Returns true if carryover status might change:
            //      * Value goes from zero to non-zero
            //      * Value changes sign
            bool setToZero(const Coord& xy);
            protected: bool locked_setToZero(const Coord& xy); public:

            // Set the value at coords to zero, returns subset containing those whose values changed
            CoordSet setToZero(const CoordSet& coords);
            protected: CoordSet locked_setToZero(const CoordSet& purgeSet); public:

            // Changes the value at xy
            //  Returns true if carryover status might change:
            //      * Value goes from zero to non-zero
            //      * Value changes sign
            bool setValue(const Coord& xy, Digit value);
            bool setValue(const Coord& xy, int value);
            bool setValue(const Coord& xy, long value);
            bool setValue(const Coord& xy, long long value);
            protected:
                bool locked_setValue(const Coord& xy, Digit value);
                bool locked_setValue(const Coord& xy, int value);
                bool locked_setValue(const Coord& xy, long value);
                bool locked_setValue(const Coord& xy, long long value);
            public:


        // *** Conversion / display

            // Converts the MDN to a human-readable string.
            std::string toString() const;
            protected: std::string locked_toString() const; public:

            // Converts the MDN to an array of strings, representing rows
            std::vector<std::string> toStringRows() const;
            protected: std::vector<std::string> locked_toStringRows() const; public:

            // Converts the MDN to an array of strings, representing columns
            std::vector<std::string> toStringCols() const;
            protected: std::vector<std::string> locked_toStringCols() const; public:


        // *** Transformations

            // Clears all addressing and bounds data, and rebuilds it
            void rebuildMetadata() const;
            protected: void locked_rebuildMetadata() const; public:

            // Returns true if m_bounds are both valid, finite numbers
            bool hasBounds() const;
            protected: bool locked_hasBounds() const; public:

            // Retuns bounds of non zero entries in m_raw
            const Rect& bounds() const;
            protected: const Rect& locked_getBounds() const; public:


        // *** Other functionality

            int getPrecision() const;
            protected: int locked_getPrecision() const; public:

            // Change the setting for m_precision, returns the number of dropped digits
            int setPrecision(int newMaxSpan);
            protected: int locked_setPrecision(int newMaxSpan); public:

            // Query the precision status of xy to ensure precision is not exceeded
            // Returns:
            //  * PrecisionStatus::Below  - above precision window
            //  * PrecisionStatus::Inside - within precision window
            //  * PrecisionStatus::Above  - below precision window
            PrecisionStatus checkPrecisionWindow(const Coord& xy) const;
            protected: PrecisionStatus locked_checkPrecisionWindow(const Coord& xy) const; public:

            // Register class as observer, inform when destroyed
            void registerObserver(MdnObserver* obs) const;
            protected: void locked_registerObserver(MdnObserver* obs) const;

            // Set the m_modified flag to trigger housekeeping with derived data when operations are
            // complete

            // Use when the data has changed, but operation may not yet be complete
            void internal_modified();

            // Use when the operation is complete, but the data may not have changed
            void internal_operationComplete();

            // Use when the data has changed AND the operation is complete, performs housekeeping:
            //  * clear derived data
            //  * increment m_event flag
            void internal_modifiedAndComplete();


protected:

    // *** Private Member Functions

        // Lock m_mutex for writeable reasons (unique_lock)
        WritableLock lockWriteable() const;

        // Lock m_mutex for read-only reasons (shareable_lock)
        ReadOnlyLock lockReadOnly() const;

        // Ensure the supplied argument 'that' is not the same as *this, throws if not
        void assertNotSelf(Mdn2dBase& that, const std::string& description) const;


        // *** Internal functions
        // All these functions require the mutex to be locked first before calling

            // Clears all addressing and bounds data
            virtual void internal_clearMetadata() const;

            // Sets value at xy without checking in range of base
            //  Returns true if carryover status might change:
            //      * Value goes from zero to non-zero
            //      * Value changes sign
            bool internal_setValueRaw(const Coord& xy, Digit value);

            // Add xy as a non-zero number position to the metadata
            void internal_insertAddress(const Coord& xy) const;

            // Checks if value is within +/- (m_base-1).  If not, throws or returns false.
            template <class Type>
            void internal_checkDigit(const Coord& xy, Type value) const {
                Digit baseDigit = m_config.baseDigit();

                if (Log_Showing_Debug4) {
                    Log_N_Debug4(
                        "Checking value " << static_cast<int>(value) << " against base "
                        << int(baseDigit)
                    );
                }
                if (value >= baseDigit || value <= -baseDigit) {
                    OutOfRange err(xy, static_cast<int>(value), baseDigit);
                    Log_N_Error(err.what());
                    throw err;
                }
            }

            // Purge any digits that exceed the precision window, return the number of purged digits
            int internal_purgeExcessDigits();

            // Update the m_bounds based on the current values
            void internal_updateBounds();

};

} // end namespace mdn
----- Mdn2dConfig.h -----
#pragma once

#include <cmath>
#include <sstream>

#include "Constants.h"
#include "Digit.h"
#include "Fraxis.h"
#include "GlobalConfig.h"
#include "Logger.h"
#include "MdnException.h"
#include "Mdn2dFramework.h"
#include "SignConvention.h"

namespace mdn {



// Contains all settings governing behaviour of an Mdn2d
class MDN_API Mdn2dConfig {

    // Calculate minimum fraction value to add to a digit, that it will appear as a non-zero
    // digit within m_precision
    static double static_calculateEpsilon(int precisionIn, int baseIn) {
        return pow((1.0 / baseIn), (precisionIn + 1));
    }

    // Pointer to framework governing class - what object holds this Mdn2d?
    static Mdn2dFramework* m_masterPtr;


public:

    static Mdn2dFramework& master() { return *m_masterPtr; }

    // Set master pointer to the given framework, posts warning if already set
    static void setMaster(Mdn2dFramework& framework);

    // Set master pointer to the given framework, overwrites previously existing setting silently
    static void resetMaster(Mdn2dFramework& framework);

    // If a message exists here, the associated Mdn2d is invalid for the reason contained in the
    // string.  Purpose of this is to prevent throwing during move ctor, so normal operation can be
    // optimised. Edge cases that invalidate the number show up here.
    mutable std::string m_invalidReason;

    // Numerical base, beyond which no digit's magnitude can go
    int m_base;

    // Convenience - base expressed as a Digit type
    Digit m_baseDigit;

    // Convenience - base expressed as a double type
    double m_baseDouble;

    // Maximum number of digits from lowest magnitude to maximum magnitude
    int m_precision;

    // Smallest value added to a digit that can cascade to a non-zero value within the precision
    //  window
    double m_epsilon;

    // Default sign convention for polymorphic numbers
    SignConvention m_signConvention;

    // Maximum number of outerloops allowed in attempting to reach polymorphic stability
    //  That is, all required carryovers are done, and polymorphic carryovers meet the default sign
    //  convention.  Use -1 for infinite loops allowed.
    int m_maxCarryoverIters;

    // Affects 1) fractional addition, 2) divide direction
    Fraxis m_defaultFraxis;


public:

    // Summon a purely default Mdn2dConfig object
    static Mdn2dConfig static_defaultConfig() { return Mdn2dConfig(); }


    // *** Constructors

    // Construct from parts, or null
    Mdn2dConfig(
        int baseIn=10,
        int maxSpanIn=16,
        SignConvention signConventionIn=SignConvention::Positive,
        int maxCarryoverItersIn = 20,
        Fraxis defaultFraxisIn=Fraxis::X
    );

    // Returns true if the number became invalid during a noexcept function
    bool valid() { return m_invalidReason.empty(); }

    // Returns false if the number became invalid during a noexcept function
    bool invalid() { return !valid(); }

    // Returns the reason this number became invalid during a noexcept function
    std::string invalidReason() { return m_invalidReason; }

    // Set this number to invalid - required to preserve nexcept optimisations, but also handle edge
    // cases that would have thrown
    void setInvalid(const std::string& reason) { m_invalidReason = reason; }

    // Return the base
    int base() const { return m_base; }

    // Return the base as a Digit type
    Digit baseDigit() const { return m_baseDigit; }

    // Return the base as a double type
    double baseDouble() const { return m_baseDouble; }

    // Return the numeric precision
    int precision() const { return m_precision; }

    // Change the precision, update downstream derived values
    void setPrecision(int precisionIn);

    // Return the derived epsilon value
    double epsilon() const { return m_epsilon; }

    SignConvention signConvention() const { return m_signConvention; }
    void setSignConvention(int newVal);
    void setSignConvention(std::string newName) {
        m_signConvention = NameToSignConvention(newName);
    }
    void setSignConvention(SignConvention signConventionIn) {
        m_signConvention = signConventionIn;
    }

    int maxCarryoverIters() { return m_maxCarryoverIters; }
    void setMaxCarryoverIters(int newVal) {
        m_maxCarryoverIters = newVal < 0 ? constants::intMax : newVal;
    }

    Fraxis defaultFraxis() const { return m_defaultFraxis; }
    void setDefaultFraxis(int newVal);
    void setDefaultFraxis(std::string newName) { m_defaultFraxis = NameToFraxis(newName); }
    void setDefaultFraxis(Fraxis fraxisIn) { m_defaultFraxis = fraxisIn; }

    // Returns true if all settings are valid, false if something failed
    bool checkConfig() const;

    // Throws if any setting is invalid
    void validateConfig() const;

    std::string to_string() const;

    // string format: (b:10, p:16, s:Positive, c:20, f:X)
    friend std::ostream& operator<<(std::ostream& os, const Mdn2dConfig& c) {
        return os
            << "("
                << "b:" << c.m_base
                << ", p:" << c.m_precision
                << ", e:" << c.m_epsilon
                << ", s:" << SignConventionToName(c.m_signConvention)
                << ", c:" << c.m_maxCarryoverIters
                << ", f:" << FraxisToName(c.m_defaultFraxis)
            << ")";
    }

    friend std::istream& operator>>(std::istream& is, Mdn2dConfig& c) {
        // TODO - problem, epsilon is derived from precision, should not be independent
        char lparen, letter, colon, comma, rparen;
        std::string fname;
        std::string sname;
        // (b:10, p:16, e:0.0000024, s:Positive, c:20, f:X)
        // Reading [(b:10]
        is >> lparen >> letter >> colon >> c.m_base;
        // Reading [, p:16]
        is >> comma >> letter >> colon >> c.m_precision;
        // Reading [, e:2.4e-6]
        is >> comma >> letter >> colon >> c.m_epsilon;
        // Reading [, s:Positive,]
        while (letter != ',') {
            is >> letter;
            sname += letter;
        }
        c.m_signConvention = NameToSignConvention(sname);
        //Reading [ c:20]
        is >> letter >> colon >> c.m_maxCarryoverIters;
        //Reading [, f:X])
        while (letter != ')') {
            is >> letter;
            fname += letter;
        }
        c.m_defaultFraxis = NameToFraxis(fname);
        return is;
    }

    // SignConvention m_signConvention;
    // int m_maxCarryoverIters;
    // Fraxis m_defaultFraxis;

        // From the perspective of an Mdn2d, look for compatibility.  The data that matters are:
        //  base, precision, sign convention
        bool operator==(const Mdn2dConfig& rhs) const;
        bool operator!=(const Mdn2dConfig& rhs) const;
};

} // end namespace mdn
----- Mdn2dConfigImpact.h -----
#pragma once

#include <vector>
#include <string>

#include "Logger.h"

// Sign enumeration

namespace mdn {

enum class Mdn2dConfigImpact {
    Unknown,
    NoImpact,
    AllDigitsCleared,
    PossibleDigitLoss,
    PossiblePolymorphism,
    PossibleDigitLossAndPolymorphism
};

const std::vector<std::string> Mdn2dConfigImpactNames(
    {
        "Unknown",
        "NoImpact",
        "AllDigitsCleared",
        "PossibleDigitLoss",
        "PossiblePolymorphism",
        "PossibleDigitLossAndPolymorphism"
    }
);

const std::vector<std::string> Mdn2dConfigImpactDescriptions(
    {
        "Unknown",
        "No impact",
        "Total loss - all digits will be cleared",
        "Possible loss of some digits",
        "Possible change in appearance but not value (polymorphism)",
        "Possible loss of some digits and a possible change in appearance"
    }
);

inline std::string Mdn2dConfigImpactToName(Mdn2dConfigImpact Mdn2dConfigImpact) {
    int fi = int(Mdn2dConfigImpact);
    return Mdn2dConfigImpactNames[fi];
}

inline std::string Mdn2dConfigImpactToDescription(Mdn2dConfigImpact Mdn2dConfigImpact) {
    int fi = int(Mdn2dConfigImpact);
    return Mdn2dConfigImpactDescriptions[fi];
}

inline Mdn2dConfigImpact NameToMdn2dConfigImpact(const std::string& name) {
    for (int i = 0; i < Mdn2dConfigImpactNames.size(); ++i) {
        if (Mdn2dConfigImpactNames[i] == name) {
            return static_cast<Mdn2dConfigImpact>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid Mdn2dConfigImpact type: " << name << " expecting:" << std::endl;
    if (Mdn2dConfigImpactNames.size()) {
        oss << Mdn2dConfigImpactNames[0];
    }
    for (auto iter = Mdn2dConfigImpactNames.cbegin() + 1; iter != Mdn2dConfigImpactNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
----- Mdn2dFramework.h -----
#pragma once

#include "../library/GlobalConfig.h"

// Interface class for Mdn2d number framework class, for Mdn2d to talk upwards to the framework
// within which they exist

namespace mdn {

class MDN_API Mdn2dFramework {

public:

    // Returns the framework's derived class type name as a string
    virtual std::string className() const {
        return "Mdn2dFramework";
    }

    // Returns the framework's 'name', used in error messaging
    virtual std::string name() const {
        return "DummyFramework";
    }

    // Returns the framework's 'name', used in error messaging
    virtual void setName(const std::string& nameIn) {
        // Do nothing
    }

    // Returns true if an Mdn2d exists with the given name, false otherwise
    virtual bool mdnNameExists(const std::string& nameIn) const {
        // default allows name collisions
        return false;
    }

    // Gives framework final say over name changes - given a desired Mdn name change from origName
    //  to newName, returns the allowed name
    virtual std::string requestMdnNameChange(
        const std::string& origName,
        const std::string& newName
    ) {
        // Default always allows the name change
        return newName;
    }

};

static Mdn2dFramework DummyFramework;

} // end namespace mdn----- Mdn2dRules.h -----
#pragma once

#include "Carryover.h"
#include "GlobalConfig.h"
#include "Mdn2dBase.h"

namespace mdn {

// Basic rules layer of 2d multi dimensional numbers, establishes:
//  * carryover rules
//  * polymorphism
//  * equality operator, now available with polymorphism
//  * digit shifting
//  * transposing, because why not?
class MDN_API Mdn2dRules : public Mdn2dBase {

protected:

    // Polymorphic nodes and the last time it was calculated
    mutable CoordSet m_polymorphicNodes;
    mutable long long m_polymorphicNodes_event;


public:

    // Check for the type of carryover, given the pivot digit and x and y axial digits
    static Carryover static_checkCarryover(Digit p, Digit x, Digit y, Digit base);


    // *** Constructors

        // Construct null
        Mdn2dRules(std::string nameIn="");

        // Construct from a configuration
        Mdn2dRules(Mdn2dConfig config, std::string nameIn="");


        // *** Rule of five

            // Copy constructor
            Mdn2dRules(const Mdn2dRules& other, std::string nameIn="");

            // Assignment operator
            Mdn2dRules& operator=(const Mdn2dRules& other);

            // Move operator
            Mdn2dRules(Mdn2dRules&& other, std::string nameIn="") noexcept;

            // Move assignment operator
            Mdn2dRules& operator=(Mdn2dRules&& other) noexcept;

            // Virtual destructor
            virtual ~Mdn2dRules() {}


    // *** Member Functions

        // *** Config

            // Change the config - can lead to any of the Mdn2dConfigImpact effects
            // Overriding virtual fn from base for access to carryover functionality
            protected: void locked_setConfig(Mdn2dConfig newConfig) override; public:


        // *** Transformations

            // Return the Carryover type (enumareation) of xy
            Carryover checkCarryover(const Coord& xy) const;
            protected: Carryover locked_checkCarryover(const Coord& xy) const; public:

            // Perform a manual carry-over at coordinate (x, y), returns affected coords as a set
            CoordSet carryover(const Coord& xy);
            protected:
                // Internal function takes arg 'carry' to be added to root of carryover, xy
                CoordSet locked_carryover(const Coord& xy, int carry = 0);
            public:

            // Given set of suspicious coords, check if any need carryovers, and if so, do them
            //  Returns set of coordinates that actually have changed
            CoordSet carryoverCleanup(const CoordSet& coords);
            protected: CoordSet locked_carryoverCleanup(const CoordSet& coords); public:

            // Given all the non-zero coords, check if any need carryovers, and if so, do them
            //  Returns set of coordinates that actually have changed
            CoordSet carryoverCleanupAll();
            protected: CoordSet locked_carryoverCleanupAll(); public:

            // General shift interface
            void shift(int xDigits, int yDigits);
            void shift(const Coord& xy);
            protected:
                void locked_shift(const Coord& xy);
                void locked_shift(int xDigits, int yDigits);
            public:

            // Shift all digits in a direction (R=+X, L=-X, U=+Y, D=-Y)
            void shiftRight(int nDigits);
            protected: void locked_shiftRight(int nDigits); public:
            void shiftLeft(int nDigits);
            protected: void locked_shiftLeft(int nDigits); public:
            void shiftUp(int nDigits);
            protected: void locked_shiftUp(int nDigits); public:
            void shiftDown(int nDigits);
            protected: void locked_shiftDown(int nDigits); public:

            // Swap X and Y
            void transpose();
            protected: void locked_transpose(); public:

            // Returns polymorphic nodes, demand driven
            const CoordSet& getPolymorphicNodes() const;
            protected: const CoordSet& locked_getPolymorphicNodes() const; public:


protected:

    // *** Protected Member Functions

        // Find all the 'Optional' carryovers, create m_polymorphicNodes data
        void internal_polymorphicScan() const;

        // Perform a blind single carryover at xy without any checks
        void internal_oneCarryover(const Coord& xy);

        // Perform a carryover during math operations (xy magnitude must exceed base)
        void internal_ncarryover(const Coord& xy);

        // Clear all derived data, including polymorphism-related data
        virtual void internal_clearMetadata() const override;

};

} // end namespace mdn

----- MdnException.h -----
// This header defines error-handling strategy for MDN operations
#pragma once

#include <stdexcept>
#include <string>
#include "Coord.h"

namespace mdn {

// Base class for all MDN-related runtime errors
class MDN_API MdnException : public std::runtime_error {
public:
    explicit MdnException(const std::string& msg) : std::runtime_error(msg) {}
};

// Attempt to carry over from a zero digit
class MDN_API FailedAssertion : public MdnException {
public:
    FailedAssertion(const std::string& description)
        : MdnException(description) {}
};

// Attempt to carry over from a zero digit
class MDN_API InvalidCarryOver : public MdnException {
public:
    InvalidCarryOver(const Coord& xy)
        : MdnException("Coordinate " + xy.to_string() + ": invalid carry over at zero digit.") {}
};

// Attempt to assign out-of-range value to digit
class MDN_API OutOfRange : public MdnException {
public:
    OutOfRange(const Coord& xy, int value, int base)
        : MdnException(
            "Coordinate " + xy.to_string() + ": out-of-range value " + std::to_string(value)
            + ", expecting ±" + std::to_string(base-1)
        ) {}
};

// Attempt to assign out-of-range value to digit
class MDN_API ZeroEncountered : public MdnException {
public:
    ZeroEncountered(const Coord& xy)
        : MdnException(
            "Coordinate " + xy.to_string() + ": has recorded value of 0.  Sparse storage should " +
            "never contain a zero."
        ) {}
};

// MetaData is out of sync or invalid
class MDN_API MetaDataInvalid : public MdnException {
public:
    MetaDataInvalid(const std::string& description)
        : MdnException("MDN MetaData invalid: " + description) {};
};

// Argument cannot be self
class MDN_API IllegalSelfReference : public MdnException {
public:
    IllegalSelfReference(const std::string& description)
        : MdnException("Supplied argument must not be the exact same object: " + description) {};
};

// Bases must match
class MDN_API BaseMismatch : public MdnException {
public:
    BaseMismatch(int baseA, int baseB)
        : MdnException(
            "Bases of interacting Mdn2d objects must match: (" + std::to_string(baseA) +
            " != " + std::to_string(baseB) + ")"
        ) {};
};

// This operation breaks the governing laws of MDNs
class MDN_API IllegalOperation : public MdnException {
public:
    IllegalOperation(const std::string& description)
        : MdnException("Illegal operation: " + description) {};
};

// The objects or data have encountered an invalid state
class MDN_API InvalidState: public MdnException {
public:
    InvalidState(const std::string& description)
        : MdnException("Invalid state detected: " + description) {};
};

// Division by zero is about to occur
class MDN_API DivideByZero : public MdnException {
public:
    DivideByZero()
        : MdnException("Division by zero") {};
};

// Argument is not valid
class MDN_API InvalidArgument : public MdnException {
public:
    InvalidArgument(const std::string& description)
        : MdnException("Invalid argument detected: " + description) {};
};

// Cannot perform the operation for the given described reason
class MDN_API InvalidOperation : public MdnException {
public:
    InvalidOperation(const std::string& description)
        : MdnException("Invalid operation detected: " + description) {};
};


} // namespace mdn
----- MdnObserver.h -----
#pragma once

#include "Logger.h"

namespace mdn {

class Mdn2d;

// Hold a reference to an Mdn2d, and observe it for changes to stay up-to-date
class MdnObserver {

protected:

    // The object I am observing
    mutable Mdn2d* m_ref = nullptr;

    int m_instance = -1;

public:

    // Return instance of this observer
    int instance() const {
        return m_instance;
    }

    // Set the instance of this observer
    int setInstance(int instance) {
        #ifdef MDN_DEBUG
            if (m_instance >= 0) {
                Log_Warn("Setting MdnObserver instance when it is already set.");
            }
        #endif
        m_instance = instance;
    }

    // Set the reference Mdn2d
    virtual void set(Mdn2d* ref) {
        m_ref = ref;
    }

    // Return the reference Mdn2d
    virtual Mdn2d* get() const {
        return m_ref;
    }

    // The observed object has been modified
    virtual void modified() const {}

    // The observed object is being reallocated to a new address
    virtual void reallocating(Mdn2d* newRef) {
        m_ref = newRef;
    }

    // The observed object is being destroyed
    virtual void farewell() {
        m_ref = nullptr;
    }

};

} // End namespace mdn
----- NamedEnum.h -----
#pragma once

#include <vector>
#include <string>

#include "Logger.h"

// A few general tools to convert templated Enum into its associated name, and visa-versa
// Not actually used yet.

namespace mdn {

template<class Enum>
std::string EnumToName(Enum e, const std::vector<std::string>& names) {
    int ei = int(e);
    return names[ei];
}

template<class Enum>
Enum NameToEnum(const std::string& name, std::vector<std::string>& names) {
    for (int i = 0; i < names.size(); ++i) {
        if (names[i] == name) {
            return static_cast<Enum>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid Enum type: " << name << " expecting:" << std::endl;
    if (names.size()) {
        oss << names[0];
    }
    for (auto iter = names.cbegin() + 1; iter != names.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
----- PolymorphicTree.h -----
#pragma once

/*
See also Carryover.h

This file is me trying to figure out ways to fully capture all polymorphic states of an MDN.
The current library does not use any of this.  Consider this just a collection of notes.

Polymorphic data structure (pmds)
We start with an Mdn in a valid state.  All required c/o's have been performed.  The polymorphic
data is probably going to be a tree.  Top level nodes are all optional c/o's.

[0,0] [2,0] [0,4] [1,5] ...
  |     |     |     |

Below each one are downstream c/o's that may be altered / awakened / removed, for example:

[0,0]
    * [0,1] - from Optional --> Invalid
    * [1,0] - from Invalid --> Required

Any newly created 'Required' c/o's must be actuated during the construction of the pmds.

[0,0]
    * [0,1] -Optional
    * [1,0] +Required --> [2,0] no change (invalid in this example)
                      \-> [1,1] +Optional

Some 'optional' nodes can appear more than once.

Static representation
    Coord
        Toggles [c0, c1, ...]


Node:

    Coord (x, y)
        The coordinates of the polymorphic carryover / optional carryover
    Downstream  [c0, c1, ...]
        The downstream required c/o's
    Activates [c0, c1, ...]
        What carryovers it activates (original coord and downstream)
    Deactivates [c0, c1, ...]
        What carryovers it deactivates

    Map (Coord)
        How to get to carryover this digit, if at all
    Activates [[c0, c1, ...],[c0, c1, ...]]
        Each list of coords is a list of all necessary c/o's needed before reaching Coord
        Multiple lists because sometimes more than one route leads to the same c/o

    Straight up, honest tree
    C0-,-C1-C3
       '-C2
*/

#include <unordered_map>
#include <utility>

#include "Coord.h"

namespace mdn {

struct PolymorphicTree {
    std::unordered_map<Coord, PolymorphicTreeNode> m_tree;
    std::unordered_set<Coord> m_currentIndex;
    std::vector<PolymorphicTreeNode> m_breadcrumbs;

    void clear() {
        m_tree.clear();
        m_currentIndex.clear();
        m_breadcrumbs.clear();
    }
};

struct PolymorphicTreeNode {
    Coord m_root;
    std::vector<PolymorphicTreeNode> m_deactivations;
    std::vector<PolymorphicTreeNode> m_activations;
};



enum class Carryover {
    Invalid,            // Valid MDN --> carryover --> Invalid MDN
    OptionalPositive,   // Valid MDN --> carryover --> Valid MDN, current value positive
    OptionalNegative,   // Valid MDN --> carryover --> Valid MDN, current value negative
    Required            // Invalid MDN --> carryover --> Valid MDN
};

} // end namespace----- PrecisionStatus.h -----
#pragma once

// Precision status
//  Is this coordinate within the precision window; if not is it above or below?

#include <vector>
#include <string>

#include "Logger.h"

namespace mdn {

enum class PrecisionStatus {
    Below,
    Inside,
    Above
};

const std::vector<std::string> PrecisionStatusNames(
    {
        "Below",
        "Inside",
        "Above"
    }
);

inline std::string PrecisionStatusToName(PrecisionStatus precisionStatus) {
    int fi = int(precisionStatus);
    // #ifdef MDN_DEBUG
    //     if (fi < 0 || fi >= PrecisionStatusNames.size())
    //         Logger::instance().error("PrecisionStatus out of range: " + std::to_string(fi));
    // #endif
    return PrecisionStatusNames[fi];
}

inline PrecisionStatus NameToPrecisionStatus(const std::string& name) {
    for (int i = 0; i < PrecisionStatusNames.size(); ++i) {
        if (PrecisionStatusNames[i] == name) {
            return static_cast<PrecisionStatus>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid PrecisionStatus type: " << name << " expecting:" << std::endl;
    if (PrecisionStatusNames.size()) {
        oss << PrecisionStatusNames[0];
    }
    for (auto iter = PrecisionStatusNames.cbegin() + 1; iter != PrecisionStatusNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
----- Rect.h -----
#pragma once

#include "Coord.h"
#include "GlobalConfig.h"
#include <vector>
#include <algorithm>
#include <limits>

namespace mdn {

class MDN_API Rect {

public:

    // *** Static member functions

    // Return intersection of two rects. If they don't intersect, returns Rect::invalid()
    inline static Rect intersection(const Rect& a, const Rect& b) {
        Coord minPt(
            std::max(a.min().x(), b.min().x()),
            std::max(a.min().y(), b.min().y())
        );
        Coord maxPt(
            std::min(a.max().x(), b.max().x()),
            std::min(a.max().y(), b.max().y())
        );

        Rect result(minPt, maxPt);
        return result.isValid() ? result : Rect::invalid();
    }

    // Return the smallest rect that contains both input rects
    inline static Rect unionOf(const Rect& a, const Rect& b) {
        if (!a.isValid()) return b;
        if (!b.isValid()) return a;

        Coord minPt(
            std::min(a.min().x(), b.min().x()),
            std::min(a.min().y(), b.min().y())
        );
        Coord maxPt(
            std::max(a.max().x(), b.max().x()),
            std::max(a.max().y(), b.max().y())
        );

        return Rect(minPt, maxPt);
    }

    // Check if rects overlap (intersection is non-empty)
    inline static bool overlaps(const Rect& a, const Rect& b) {
        return intersection(a, b).isValid();
    }

    // Check if two rects are adjacent but not overlapping
    inline static bool areAdjacent(const Rect& a, const Rect& b) {
        if (overlaps(a, b)) return false;

        // One of the edges must touch
        bool horizontalTouch =
            (a.right() + 1 == b.left() || b.right() + 1 == a.left()) &&
            !(a.top() < b.bottom() || b.top() < a.bottom());

        bool verticalTouch =
            (a.top() + 1 == b.bottom() || b.top() + 1 == a.bottom()) &&
            !(a.right() < b.left() || b.right() < a.left());

        return horizontalTouch || verticalTouch;
    }


    // Returns an invalid rectangle, plays nice with growToInclude
    static Rect invalid() {
        return Rect(
            Coord(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()),
            Coord(std::numeric_limits<int>::min(), std::numeric_limits<int>::min())
        );
    }


    // *** Constructors

    // Construct null - retuns an invalid rectangle
    Rect() : Rect(invalid()) {}

    // Construct from components
    //  fixOrdering - when true, corrects inputs where individual components of min and max might
    //      need to be swapped. Useful when source coords are unreliable. See fixOrdering function.
    Rect(Coord min, Coord max, bool fixOrdering = false)
        : m_min(min), m_max(max)
    {
        if (fixOrdering) {
            this->fixOrdering();
        }
    }

    // Construct from elemental components
    //  fixOrdering - when true, corrects inputs where individual components of min and max might
    //      need to be swapped. Useful when source coords are unreliable. See fixOrdering function.
    Rect(int xmin, int ymin, int xmax, int ymax, bool fixOrdering = false)
        : m_min(xmin, ymin), m_max(xmax, ymax)
    {
        if (fixOrdering) {
            this->fixOrdering();
        }
    }


    // *** Rule of five
    //  Compliant by default

        Rect(const Rect&) = default;
        Rect(Rect&&) = default;
        Rect& operator=(const Rect&) = default;
        Rect& operator=(Rect&&) = default;
        ~Rect() = default;


    // *** Public member functions

    // Returns true if this Rect is valid, i.e. min and max make sense
    bool isValid() const {
        return m_min.x() <= m_max.x() && m_min.y() <= m_max.y();
    }

    // Treats each component independently, ensures min and max are in the correct variable
    void fixOrdering() {
        if (m_min.x() > m_max.x()) std::swap(m_min.x(), m_max.x());
        if (m_min.y() > m_max.y()) std::swap(m_min.y(), m_max.y());
    }

    Coord min() const { return m_min; }
    Coord max() const { return m_max; }

    int left()   const { return m_min.x(); }
    int right()  const { return m_max.x(); }
    int bottom() const { return m_min.y(); }
    int top()    const { return m_max.y(); }

    int width()  const { return m_max.x() - m_min.x() + 1; }
    int height() const { return m_max.y() - m_min.y() + 1; }

    // A mathematical vector from min to max, gives (nColumns-1, nRows-1)
    Coord delta() const { return m_max - m_min; }

    // Count for number of elements along each dimension, gives (nColumns, nRows)
    Coord gridSize() const { return Coord(width(), height()); }

    // Returns the total number of integer Coordinates covered by this object, does not count non-
    //  zeros.
    int size() const {
        return isValid() ? width() * height() : 0;
    }

    bool isSingleCoord() const {
        return isValid() && m_min == m_max;
    }

    bool isMultiCoord() const {
        return isValid() && m_min != m_max;
    }

    bool contains(const Coord& c) const {
        return c.x() >= m_min.x() && c.x() <= m_max.x()
            && c.y() >= m_min.y() && c.y() <= m_max.y();
    }

    void clear() {
        *this = Rect::Invalid();
    }

    void set(Coord min, Coord max, bool fixOrdering = false) {
        m_min = min;
        m_max = max;
        if (fixOrdering) {
            this->fixOrdering();
        }
    }
    void set(int xmin, int ymin, int xmax, int ymax, bool fixOrdering = false) {
        m_min.set(xmin, ymin);
        m_max.set(xmax, ymax);
        if (fixOrdering) {
            this->fixOrdering();
        }
    }
    void setMin(int x, int y, bool fixOrdering = false) {
        m_min.set(x, y);
        if (fixOrdering) {
            this->fixOrdering();
        }
    }
    void setMax(int x, int y, bool fixOrdering = false) {
        m_max.set(x, y);
        if (fixOrdering) {
            this->fixOrdering();
        }
    }

    void translate(int dx, int dy) {
        m_min.translate(dx, dy);
        m_max.translate(dx, dy);
    }

    Rect translated(int dx, int dy) const {
        return Rect(m_min.translated(dx, dy), m_max.translated(dx, dy));
    }

    void growToInclude(const Coord& c) {
        if (!isValid()) {
            m_min = m_max = c;
            return;
        }
        if (c.x() < m_min.x()) m_min.x() = c.x();
        if (c.x() > m_max.x()) m_max.x() = c.x();
        if (c.y() < m_min.y()) m_min.y() = c.y();
        if (c.y() > m_max.y()) m_max.y() = c.y();
    }

    std::vector<Coord> toCoordVector() const {
        std::vector<Coord> coords;
        if (!isValid()) return coords;
        for (int y = bottom(); y <= top(); ++y) {
            for (int x = left(); x <= right(); ++x) {
                coords.emplace_back(x, y);
            }
        }
        return coords;
    }

    friend std::ostream& operator<<(std::ostream& os, const Rect& r) {
        if (!r.isValid()) {
            os << "[Empty]";
        } else {
            os << "[" << r.min() << " -> " << r.max() << "]";
        }
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Rect& r) {
        char ch;
        is >> ch;

        if (ch != '[') {
            is.setstate(std::ios::failbit);
            return is;
        }

        // Peek ahead to see if it's "Empty]"
        std::string word;
        is >> word;

        if (word == "Empty]") {
            r = Rect::Invalid();
            return is;
        }

        // Back up stream to parse normally
        is.putback(word.back());
        for (int i = static_cast<int>(word.size()) - 2; i >= 0; --i)
            is.putback(word[i]);

        Coord min, max;
        is >> min;

        is >> ch; // read '-'
        if (ch != '-') {
            is.setstate(std::ios::failbit);
            return is;
        }

        is >> ch; // expect '>'
        if (ch != '>') {
            is.setstate(std::ios::failbit);
            return is;
        }

        is >> max;

        is >> ch; // read ']'
        if (ch != ']') {
            is.setstate(std::ios::failbit);
            return is;
        }

        r = Rect(min, max);
        return is;
    }


private:

    Coord m_min;
    Coord m_max;
};

} // end namespace mdn
----- SignConvention.h -----
#pragma once

#include <vector>
#include <string>

#include "Logger.h"

// Sign enumeration

namespace mdn {

enum class SignConvention {
    Invalid,
    Default,
    Positive,
    Negative
};

const std::vector<std::string> SignConventionNames(
    {
        "Invalid",
        "Default",  // Or, do not change
        "Positive",
        "Negative"
    }
);

inline std::string SignConventionToName(SignConvention SignConvention) {
    int fi = int(SignConvention);
    // #ifdef MDN_DEBUG
    //     if (fi < 0 || fi >= SignConventionNames.size())
    //         Logger::instance().error("SignConvention out of range: " + std::to_string(fi));
    // #endif
    return SignConventionNames[fi];
}

inline SignConvention NameToSignConvention(const std::string& name) {
    for (int i = 0; i < SignConventionNames.size(); ++i) {
        if (SignConventionNames[i] == name) {
            return static_cast<SignConvention>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid SignConvention type: " << name << " expecting:" << std::endl;
    if (SignConventionNames.size()) {
        oss << SignConventionNames[0];
    }
    for (auto iter = SignConventionNames.cbegin() + 1; iter != SignConventionNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
----- Tools.h -----
#pragma once

#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Constants.h"
#include "Digit.h"
#include "GlobalConfig.h"

namespace mdn {

class MDN_API Tools {

public:

    static const std::vector<char> m_digToAlpha;
    static const std::string m_boxArt_h; // ─
    static const std::string m_boxArt_v; // │
    static const std::string m_boxArt_x; // ┼

    template <class T>
    struct is_byte_like
    : std::bool_constant<
            std::is_integral_v<std::remove_cv_t<T>> &&
            sizeof(std::remove_cv_t<T>) == 1> {};
    template <class T>
    inline static constexpr bool is_byte_like_v = is_byte_like<T>::value;


    // Convert a vector of anything to a string delimiter
    template<typename T>
    static std::string vectorToString(
        const std::vector<T>& array, const std::string& delimiter, bool reverse
    ) {
        if (array.empty()) {
            return "";
        }

        std::ostringstream oss;
        if (reverse) {
            size_t lastI = array.size()-1;
            if constexpr (is_byte_like_v<T>)
                out << static_cast<int>(array[lastI]);
            else
                out << array[lastI];
            for (size_t i = lastI-1; i >= 0; --i) {
                if constexpr (is_byte_like_v<T>)
                    out << delimiter << static_cast<int>(array[i]);
                else
                    out << delimiter << array[i];
            }
        } else {
            if constexpr (is_byte_like_v<T>)
                out << static_cast<int>(array[0]);
            else
                out << array[0];
            for (size_t i = 1; i < array.size(); ++i) {
                oss << delimiter << array[i];
                if constexpr (is_byte_like_v<T>)
                    out << delimiter <<static_cast<int>(array[i]);
                else
                    out << delimiter << array[i];
            }
        }
        return oss.str();
    }

    // Optional overload for char delimiter
    template<typename T>
    static std::string vectorToString(const std::vector<T>& array, char delimiter, bool reverse) {
        return vectorToString(array, std::string(1, delimiter), reverse);
    }

    template<typename S, typename T>
    static std::string pairToString(const std::pair<S, T>& pair, const std::string& delimiter) {
        std::ostringstream oss;
        oss << pair.first << delimiter << pair.second;
        return oss.str();
    }

    // Optional overload for char delimiter
    template<typename S, typename T>
    static std::string pairToString(const std::pair<S, T>& pair, char delimiter) {
        return pairToString(pair, std::string(1, delimiter));
    }

    // Convert a set of anything to a string delimiter
    template<typename T>
    static std::string setToString(const std::unordered_set<T>& set, const std::string& delimiter) {
        if (set.empty()) return "";

        std::ostringstream oss;
        bool first = true;
        for (T elem : set) {
            if (first) {
                first = false;
                oss << elem;
            } else {
                oss << delimiter << elem;
            }
        }
        return oss.str();
    }

    // Optional overload for char delimiter
    template<typename T>
    static std::string setToString(const std::unordered_set<T>& set, char delimiter) {
        return setToString(set, std::string(1, delimiter));
    }

    // Converts a value between -32 and 32 to -V,..,-C,-B,-A,-9,-8, .. ,-1,0,1,...,8,9,A,B,...,V
    static std::string digitToAlpha(
        Digit value, std::string pos=" ", std::string neg=m_boxArt_h
    );
    static std::string digitToAlpha(
        int value, std::string pos=" ", std::string neg=m_boxArt_h
    );
    static std::string digitToAlpha(
        long value, std::string pos=" ", std::string neg=m_boxArt_h
    );
    static std::string digitToAlpha(
        long long value, std::string pos=" ", std::string neg=m_boxArt_h
    );

    // Convert a std::vector<Digit> to a string
    static std::string digitArrayToString(
        const std::vector<Digit>& array,
        char delim=',',
        char open='(',
        char close=')'
    );

    // Ensure div is not too close to zero
    static void stabilise(float& div);
    static void stabilise(double& div);

    // Get filename from full path
    static std::string removePath(const char* fullpath);

};

} // namespace mdn
----- Carryover.h -----
----- Constants.h -----
----- Coord.h -----
----- CoordSet.h -----
----- Digit.h -----
----- ErrorHandling.h -----
----- Fraxis.h -----
----- GlobalConfig.h -----
----- Logger.h -----
----- Mdn2d.h -----
----- Carryover.h -----
#pragma once

// Carryovers
//  Some carryovers are required, such as when a digit magnitude exceeds its base.  Other times a
//  carryover is optional - some MDNs have multiple valid states.  Switching between these states is
//  achieved using carryovers.  And some carryovers bring a number to an invalid state and should
//  not be performed.
//
//  In general, there are three types of carryovers:
//
//      * Invalid:    Valid MDN --> carryover --> Invalid MDN
//      * Optional:   Valid MDN --> carryover --> Valid MDN
//      * Required: Invalid MDN --> carryover --> Valid MDN
//
//  A  3    |   4
//     0 -3 | -10 -2
//
//  B  0    |   1
//     4 0  |  -6 1
//
//  C  0    |   1
//     4 3  |  -6 4
//
//  D  0    |   1
//     4 -3 |  -6 -2
//
//  E -2    |  -1
//     4  3 |  -6  4
//
//  F  2    |   3
//     4  3 |  -6  4
//
//  G -2    |  -1
//     4 -3 |  -6 -2
//
//    |  p  |  x  |  y  | comment
//    +-----+-----+-----+---------------------
// A  |  0  |  ?  |  ?  | not possible
//
// G  |  +  |  -  |  -  | required
// D  |  +  |  -  |  0  | optional
// E  |  +  |  +  |  -  | optional
// B  |  +  |  0  |  0  | invalid
// C  |  +  |  +  |  0  | invalid
// F  |  +  |  +  |  +  | invalid
//
// M  |  -  |  +  |  +  | required
// J  |  -  |  +  |  0  | optional
// K  |  -  |  -  |  +  | optional
// H  |  -  |  0  |  0  | invalid
// I  |  -  |  -  |  0  | invalid
// L  |  -  |  -  |  -  | invalid

// Interactions between polymorphic molecules
//
//  a  3    |   3      a'
//     0 -3 |   1 -3
//     2 -3 |  -8 -2
//
// In [a] above:
//  * the [2] is a polymorphic node, and
//  * the [0] is an invalid carryover
// However, in [a'] above:
//  * the [-8] is a polymorphic node (still), but
//  * the [1] is now a newly created polymorphic node
//
// Let's consider the [0] in [a] to be a dormant polymorphic node.  A polymorphic carryover can
// awaken a dormant polymorphic node.
//
//  b  3    |   3      b'
//     0 -3 |   1 -3
//     2 -3 |  -8 -2
//
//
// Further testing shows that a carryover can alter the neighbouring digit's carryover status, at
// any time.

#include <vector>
#include <string>

#include "Logger.h"

// Carryover
//  Type of carryover, depending on the root digit and the axial digits

namespace mdn {

enum class Carryover {
    Invalid,            // Valid MDN --> carryover --> Invalid MDN
    OptionalPositive,   // Valid MDN --> carryover --> Valid MDN, current value positive
    OptionalNegative,   // Valid MDN --> carryover --> Valid MDN, current value negative
    Required            // Invalid MDN --> carryover --> Valid MDN
};

const std::vector<std::string> CarryoverNames(
    {
        "Invalid",
        "OptionalPositive",
        "OptionalNegative",
        "Required"
    }
);

inline std::string CarryoverToName(Carryover carryover) {
    int fi = int(carryover);
    // #ifdef MDN_DEBUG
    //     if (fi < 0 || fi >= CarryoverNames.size())
    //         Logger::instance().error("Carryover out of range: " + std::to_string(fi));
    // #endif
    return CarryoverNames[fi];
}

inline Carryover NameToCarryover(const std::string& name) {
    for (int i = 0; i < CarryoverNames.size(); ++i) {
        if (CarryoverNames[i] == name) {
            return static_cast<Carryover>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid Carryover type: " << name << " expecting:" << std::endl;
    if (CarryoverNames.size()) {
        oss << CarryoverNames[0];
    }
    for (auto iter = CarryoverNames.cbegin() + 1; iter != CarryoverNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
----- Constants.h -----
#pragma once

#include <limits>

#include "Digit.h"

namespace mdn {

namespace constants {

constexpr int intMin = std::numeric_limits<int>::min();
constexpr int intMax = std::numeric_limits<int>::max();
constexpr Digit DigitMin = std::numeric_limits<Digit>::min();
constexpr Digit DigitMax = std::numeric_limits<Digit>::max();

constexpr float floatSmall = 1e-6;
constexpr double doubleSmall = 1e-12;

} // end namspace constants

} // end namespace mdn
----- Coord.h -----
#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <functional>
#include <tuple>

#include "GlobalConfig.h"

namespace mdn {

class MDN_API Coord {
public:
    constexpr Coord(int x = 0, int y = 0) : m_x(x), m_y(y) {}

    // Element accessors
    int x() const { return m_x; }
    int& x() { return m_x; }
    int y() const { return m_y; }
    int& y() { return m_y; }

    void set(int x, int y) {
        m_x = x;
        m_y = y;
    }

    // *** Functions that create a new Coord instance

        // Creates a new Coord, translated by the given (xIncr, yIncr)
        Coord translated(int xIncr, int yIncr) const {
            Coord ret(*this);
            ret.m_x += xIncr;
            ret.m_y += yIncr;
            return ret;
        }
        // Creates a new Coord, translated along x by the given xIncr
        Coord translatedX(int xIncr) const {
            Coord ret(*this);
            ret.m_x += xIncr;
            return ret;
        }
        // Creates a new Coord, translated along y by the given yIncr
        Coord translatedY(int yIncr) const {
            Coord ret(*this);
            ret.m_y += yIncr;
            return ret;
        }


    // *** Functions that modify *this

        // Move the Coord by the given (xIncr, yIncr)
        void translate(int xIncr, int yIncr) { m_x += xIncr; m_y += yIncr; }
        // Move the Coord along x by the given xIncr
        void translateX(int xIncr) { m_x += xIncr; }
        // Move the Coord along y by the given yIncr
        void translateY(int yIncr) { m_y += yIncr; }

    // Compound operators
    Coord& operator+=(int val) {
        m_x += val;
        m_y += val;
        return *this;
    }

    Coord& operator*=(int val) {
        m_x *= val;
        m_y *= val;
        return *this;
    }

    Coord& operator/=(int val) {
        m_x /= val;
        m_y /= val;
        return *this;
    }

    Coord& operator-=(int val) {
        m_x -= val;
        m_y -= val;
        return *this;
    }

    Coord& operator+=(const Coord& other) {
        m_x += other.m_x;
        m_y += other.m_y;
        return *this;
    }

    Coord& operator-=(const Coord& other) {
        m_x -= other.m_x;
        m_y -= other.m_y;
        return *this;
    }

    // Arithmetic operators
    Coord operator+(int val) const {
        return Coord(*this) += val;
    }

    Coord operator-(int val) const {
        return Coord(*this) -= val;
    }

    Coord operator*(int val) const {
        return Coord(*this) *= val;
    }

    Coord operator/(int val) const {
        return Coord(*this) /= val;
    }

    Coord operator+(const Coord& other) const {
        return Coord(*this) += other;
    }

    Coord operator-(const Coord& other) const {
        return Coord(*this) -= other;
    }

    bool operator==(const Coord& other) const {
        return m_x == other.m_x && m_y == other.m_y;
    }

    bool operator!=(const Coord& other) const {
        return !(*this == other);
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const Coord& c) {
        return os << "(" << c.m_x << ", " << c.m_y << ")";
    }

    friend std::istream& operator>>(std::istream& is, Coord& c) {
        char lparen, comma, rparen;
        return (is >> lparen >> c.m_x >> comma >> c.m_y >> rparen);
    }

    // Structured binding support
    friend constexpr int get(const Coord& c, std::integral_constant<std::size_t, 0>) noexcept {
        return c.m_x;
    }
    friend constexpr int get(const Coord& c, std::integral_constant<std::size_t, 1>) noexcept {
        return c.m_y;
    }


private:
    int m_x, m_y;
};

constexpr Coord COORD_ORIGIN = Coord{0, 0};

} // namespace mdn

namespace std {
    template <>
    struct hash<mdn::Coord> {
        std::size_t operator()(const mdn::Coord& c) const noexcept {
            std::size_t h1 = std::hash<int>{}(c.x());
            std::size_t h2 = std::hash<int>{}(c.y());
            return h1 ^ (h2 << 1);
        }
    };

    template <>
    struct tuple_size<mdn::Coord> : std::integral_constant<std::size_t, 2> {};

    template <std::size_t N>
    struct tuple_element<N, mdn::Coord> {
        using type = int;
    };

    template <std::size_t N>
    constexpr int get(const mdn::Coord& c) noexcept {
        static_assert(N < 2, "Index out of range for mdn::Coord");
        if constexpr (N == 0) return c.x();
        else return c.y();
    }
}

----- CoordSet.h -----
#pragma once

#include <unordered_set>

#include "Coord.h"

namespace mdn {

using CoordSet = std::unordered_set<Coord>;

} // end namespace mdn----- Digit.h -----
#pragma once

#include <cstdint>

namespace mdn {

using Digit = int8_t;

} // end namespace mdn
----- ErrorHandling.h -----
#pragma once

#include <string>

#include "GlobalConfig.h"

namespace mdn {

class MDN_API ErrorHandling {

public:

    static void PrintStackTrace();
    static void PrintStackTrace1();

    static std::string Demangle(const char* name);

};

} // end namespace mdn----- Fraxis.h -----
#pragma once

#include <vector>
#include <string>

#include "Logger.h"

// Fraxis - fraction axis
//  The digit axis along which the fractional part of a real number expands in an MDN

namespace mdn {

enum class Fraxis {
    Invalid,
    Default,
    X,
    Y
};

const std::vector<std::string> FraxisNames(
    {
        "Invalid",
        "Default",
        "X",
        "Y"
    }
);

inline std::string FraxisToName(Fraxis fraxis) {
    int fi = int(fraxis);
    // #ifdef MDN_DEBUG
    //     if (fi < 0 || fi >= FraxisNames.size())
    //         Logger::instance().error("Fraxis out of range: " + std::to_string(fi));
    // #endif
    return FraxisNames[fi];
}

inline Fraxis NameToFraxis(const std::string& name) {
    for (int i = 0; i < FraxisNames.size(); ++i) {
        if (FraxisNames[i] == name) {
            return static_cast<Fraxis>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid Fraxis type: " << name << " expecting:" << std::endl;
    if (FraxisNames.size()) {
        oss << FraxisNames[0];
    }
    for (auto iter = FraxisNames.cbegin() + 1; iter != FraxisNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
----- GlobalConfig.h -----
#pragma once

#ifdef _WIN32
  #ifdef mdn_EXPORTS
    #define MDN_API __declspec(dllexport)
  #else
    #define MDN_API __declspec(dllimport)
  #endif
#else
  #define MDN_API
#endif


constexpr const char* past_last_slash(const char* path) {
    const char* last = path;
    while (*path) {
        if (*path == '/' || *path == '\\') last = path + 1;
        ++path;
    }
    return last;
}

#define SHORT_FILE (past_last_slash(__FILE__))

// #define SHORT_FILE ({constexpr cstr sf__ {past_last_slash(__FILE__)}; sf__;})
// extern const char* PathToPrettyName(const char*);
// static const char* TranslationUnitName(const char* Path)
// {
// static const char* Name=PathToPrettyName(Path);
// return Name;
// }
// #define FILENAME__ TranslationUnitName(__FILE__)----- Logger.h -----
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
    if (!expression) { \
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
    #define InternalLoggerNamedFileRef InternalIdentedLoggerFileRef + m_name + ": "

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
----- Mdn2d.h -----
#pragma once

#include "GlobalConfig.h"
#include "Mdn2dRules.h"

namespace mdn {

// Represents a 2D Multi-Dimensional Number (MDN).
class MDN_API Mdn2d : public Mdn2dRules {

protected:

public:

    // *** Constructors

        // Construct null
        Mdn2d(std::string nameIn="");

        // Construct from a configuration
        Mdn2d(Mdn2dConfig config, std::string nameIn="");


        // *** Rule of five

            // Copy constructor
            Mdn2d(const Mdn2d& other, std::string nameIn="");

            // Assignment operator
            Mdn2d& operator=(const Mdn2d& other);

            // Move operator
            Mdn2d(Mdn2d&& other, std::string nameIn="") noexcept;

            // Move assignment operator
            Mdn2d& operator=(Mdn2d&& other) noexcept;

            // Virtual destructor
            virtual ~Mdn2d() {}


    // *** Member Functions

        // *** Full Mdn2d mathematical operations

            // Addition: *this + rhs = ans, overwrites ans
            void plus(const Mdn2d& rhs, Mdn2d& ans) const;
            protected: CoordSet locked_plus(const Mdn2d& rhs, Mdn2d& ans) const; public:

            // Subtraction: *this - rhs = ans, overwrites ans
            void minus(const Mdn2d& rhs, Mdn2d& ans) const;
            protected: CoordSet locked_minus(const Mdn2d& rhs, Mdn2d& ans) const; public:

            // Multiplication: *this x rhs = ans, overwrites ans
            void multiply(const Mdn2d& rhs, Mdn2d& ans) const;
            protected: CoordSet locked_multiply(const Mdn2d& rhs, Mdn2d& ans) const; public:

            // Division: *this / rhs = ans, overwrites ans
            void divide(const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis=Fraxis::Default) const;
            protected: CoordSet locked_divide(
                const Mdn2d& rhs, Mdn2d& ans, Fraxis fraxis=Fraxis::Default
            ) const; public:
            protected: CoordSet locked_divideX(const Mdn2d& rhs, Mdn2d& ans) const; public:
            protected: CoordSet locked_divideY(const Mdn2d& rhs, Mdn2d& ans) const; public:


        // *** Addition / subtraction

            // Add the given number at xy, breaking into integer and fraxis operations
            void add(const Coord& xy, float realNum, Fraxis fraxis=Fraxis::Default);
            void add(const Coord& xy, double realNum, Fraxis fraxis=Fraxis::Default);
            protected: CoordSet locked_add(
                const Coord& xy, double realNum, Fraxis fraxis=Fraxis::Default
            ); public:

            // Subtract the given number at xy, breaking into integer and fraxis operations
            void subtract(const Coord& xy, float realNum, Fraxis fraxis=Fraxis::Default);
            void subtract(const Coord& xy, double realNum, Fraxis fraxis=Fraxis::Default);

            // Addition component: integer part, at xy with symmetric carryover
            void add(const Coord& xy, Digit value, Fraxis unused=Fraxis::Default);
            protected: CoordSet locked_add(const Coord& xy, Digit value); public:
            void add(const Coord& xy, int value, Fraxis unused=Fraxis::Default);
            protected: CoordSet locked_add(const Coord& xy, int value); public:
            void add(const Coord& xy, long value, Fraxis unused=Fraxis::Default);
            protected: CoordSet locked_add(const Coord& xy, long value); public:
            void add(const Coord& xy, long long value, Fraxis unused=Fraxis::Default);
            protected: CoordSet locked_add(const Coord& xy, long long value); public:

            // Subtraction component: integer part, at xy with symmetric carryover
            void subtract(const Coord& xy, Digit value, Fraxis unused=Fraxis::Default);
            void subtract(const Coord& xy, int value, Fraxis unused=Fraxis::Default);
            void subtract(const Coord& xy, long value, Fraxis unused=Fraxis::Default);
            void subtract(const Coord& xy, long long value, Fraxis unused=Fraxis::Default);

            // Addition component: fractional part, at xy with assymmetric cascade
            void addFraxis(const Coord& xy, float fraction, Fraxis fraxis=Fraxis::Default);
            void addFraxis(const Coord& xy, double fraction, Fraxis fraxis=Fraxis::Default);
            protected: CoordSet locked_addFraxis(
                const Coord& xy, double fraction, Fraxis fraxis
            ); public:

            // Subtract a fractional value cascading along the fraxis
            void subtractFraxis(const Coord& xy, float fraction, Fraxis fraxis=Fraxis::Default);
            void subtractFraxis(const Coord& xy, double fraction, Fraxis fraxis=Fraxis::Default);


        // *** Multiplication / divide

            // Multiply the full Mdn2d by an integer
            void multiply(Digit value);
            void multiply(int value);
            void multiply(long value);
            void multiply(long long value);
            protected: CoordSet locked_multiply(int value); public:
            protected: CoordSet locked_multiply(long value); public:
            protected: CoordSet locked_multiply(long long value); public:


    // *** Member Operators

        // Equality comparison
        //  The rules layer brings carryovers, allowing us to find equivalence between different
        //  states of polymorphism.  But for now, equivalence only works with a default sign
        //  convention (Mdn2dConfig)
        bool operator==(const Mdn2d& rhs) const;

        // Inequality comparison.
        bool operator!=(const Mdn2d& rhs) const;

        // Assignment addition, *this += rhs
        Mdn2d& operator+=(const Mdn2d& rhs);
        protected:
            // I'm different from unlocked: I return the changed set, but the standard += return is
            //  just *this
            CoordSet locked_plusEquals(const Mdn2d& rhs);
        public:

        // Assignment subtraction, *this -= rhs
        Mdn2d& operator-=(const Mdn2d& rhs);
        protected:
            // I'm different from unlocked: I return the changed set, but the standard -= return is
            //  just *this
            CoordSet locked_minusEquals(const Mdn2d& rhs);
        public:

        // Assignment multiplication, *this *= rhs
        Mdn2d& operator*=(const Mdn2d& rhs);
        protected:
            // I'm different from unlocked: I return the changed set, but the standard -= return is
            //  just *this
            CoordSet locked_timesEquals(const Mdn2d& rhs);
        public:

        // Placeholder for MDN division.
        Mdn2d& operator/=(const Mdn2d& rhs);

        // Scalar multiplication.
        Mdn2d& operator*=(int scalar);

        // Scalar multiplication.
        Mdn2d& operator*=(long scalar);

        // Scalar multiplication.
        Mdn2d& operator*=(long long scalar);


protected:

        // *** Internal functions
        // All these functions require the mutex to be locked first before calling

            // Apply default to fraxis as required
            void internal_checkFraxis(Fraxis& fraxis) const;

            // Execute the fraxis propagation algorithm
            //  dX, dY, c - constants to guide propagation:
            //      x Direction: -1, 0, -1
            //      y Direction: 0, -1, 1
            CoordSet internal_fraxis(const Coord& xy, double f, int dX, int dY, int c);
            CoordSet internal_fraxisCascade(const Coord& xy, Digit d, int c);

            // // plusEquals variant: *this += rhs x scalar, used in mdn x mdn algorithm
            // Mdn2d& internal_plusEquals(const Mdn2d& rhs, int scalar);

            // Creates a copy of *this and performs a multiply and shift, used in mdn x mdn
            //  return = (*this x value).shift(xy)
            Mdn2d internal_copyMultiplyAndShift(int value, const Coord& shiftXY) const;

};

// Arithmetic binary operators
inline Mdn2d operator+(Mdn2d lhs, const Mdn2d& rhs) { return lhs += rhs; }
inline Mdn2d operator-(Mdn2d lhs, const Mdn2d& rhs) { return lhs -= rhs; }
inline Mdn2d operator*(Mdn2d lhs, const Mdn2d& rhs) { return lhs *= rhs; }
inline Mdn2d operator/(Mdn2d lhs, const Mdn2d& rhs) { return lhs /= rhs; }

inline Mdn2d operator*(Mdn2d lhs, int scalar) { return lhs *= scalar; }

} // end namespace mdn
----- Mdn2dBase.h -----
#pragma once

#include <map>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "CoordSet.h"
#include "GlobalConfig.h"
#include "Mdn2dConfig.h"
#include "Mdn2dConfigImpact.h"
#include "MdnObserver.h"
#include "PrecisionStatus.h"
#include "Rect.h"

namespace mdn {

class Mdn2d;
class Project;

// Digit layer of 2d multi dimensional numbers, establishes:
//  * 2-dimensional digit representation
//  * sparse storage and addressing
//  * bounds metadata
class MDN_API Mdn2dBase {

    // *** Friends
    friend class Project;


protected:

    // Static name generation

    // Thread safety at static layer
    static std::shared_mutex m_static_mutex;

    // Used to generate the next defaulted name
    static int m_nextNameSeed;

    // Creates a new Mdn2d name, acquires static lock first
    static std::string static_generateNextName();

    // Creates a new Mdn2d name, assumes lock already acquired
    static std::string locked_generateNextName();

    // Create a 'copy' name from given nameIn (e.g. nameIn_copy0), acquires static lock first
    static std::string static_generateCopyName(const std::string& nameIn);

    // Create a 'copy' name from given nameIn (e.g. nameIn_copy0), assumes lock already acquired
    static std::string locked_generateCopyName(const std::string& nameIn);


    // *** Local variables

    // Configuration settings for this Mdn2dBase
    Mdn2dConfig m_config;

    // Thread safety
    mutable std::shared_mutex m_mutex;


    // *** Data & addressing

    // Name of this number
    std::string m_name;

    // Sparse coordinate-to-digit mapping
    std::unordered_map<Coord, Digit> m_raw;

    // Addressing
    mutable std::map<int, CoordSet> m_xIndex;
    mutable std::map<int, CoordSet> m_yIndex;

    // Full index
    mutable CoordSet m_index;

    // Observers
    mutable std::unordered_map<int, MdnObserver*> m_observers;


    // *** Metadata

        // Bounding box for non-zero digits
        mutable Rect m_bounds;

        // When true, increment m_event once operation is complete
        bool m_modified;

        // Event number for tracking derived, demand-driven data
        long long m_event;


public:

    // *** Typedefs

    using WritableLock = std::unique_lock<std::shared_mutex>;
    using ReadOnlyLock = std::shared_lock<std::shared_mutex>;


    // *** Static functions


    // Create a fully-realised Mdn2d instance, accessible from downstream layers
    static Mdn2d NewInstance(
        Mdn2dConfig config=Mdn2dConfig::static_defaultConfig(),
        std::string nameIn=""
    );
    static Mdn2d Duplicate(const Mdn2d& other, std::string nameIn="");


    // *** Constructors

        // Construct from a configuration, or default
        Mdn2dBase(std::string nameIn="");

        Mdn2dBase(Mdn2dConfig config, std::string nameIn="");


        // *** Rule of five

            // Copy constructor
            Mdn2dBase(const Mdn2dBase& other, std::string nameIn="");

            // Assignment operator
            Mdn2dBase& operator=(const Mdn2dBase& other);

            // Move operator
            Mdn2dBase(Mdn2dBase&& other, std::string nameIn="") noexcept;

            // Move assignment operator
            Mdn2dBase& operator=(Mdn2dBase&& other) noexcept;

            // Virtual destructor
            virtual ~Mdn2dBase();


    // *** Member Functions

        // *** Config

            // Return the Mdn2dConfig for this number
            const Mdn2dConfig& getConfig() const;
            protected: const Mdn2dConfig& locked_getConfig() const; public:

            // Assess the impact of changing the config to the supplied newConfig
            Mdn2dConfigImpact assessConfigChange(const Mdn2dConfig& newConfig);
            protected: Mdn2dConfigImpact locked_assessConfigChange(const Mdn2dConfig& newConfig); public:

            // Change the config - can lead to any of the Mdn2dConfigImpact effects
            void setConfig(Mdn2dConfig& newConfig);
            // Locked version *copies* config - everyone has their own copy
            protected: virtual void locked_setConfig(Mdn2dConfig newConfig); public:

        // *** Observers

            // Register a new observer
            void registerObserver(MdnObserver* obs) const;
            protected: void locked_registerObserver(MdnObserver* obs) const; public:

            // Unregister the owner (does not delete observer)
            void unregisterObserver(MdnObserver* obs) const;
            protected: void locked_unregisterObserver(MdnObserver* obs) const; public:


        // *** Identity

            // Return name
            const std::string& getName() const;
            protected: const std::string& locked_getName() const; public:

            // Set this number's 'name', deferring to framework for approval
            void setName(const std::string& nameIn);
            protected: void locked_setName(const std::string& nameIn); public:


        // *** Getters

            // Retrieves the value at coordinate (x, y), or 0 if not present.
            Digit getValue(const Coord& xy) const;
            protected: Digit locked_getValue(const Coord& xy) const; public:

            // Write out a continuous range for a row, from xStart to xEnd, inclusive.
            // out.size() becomes (x1 - x0 + 1). Returns false only on internal error.
            bool getRowRange
            (
                int y,
                int xStart,
                int xEnd,
                std::vector<mdn::Digit>& out
            ) const;
        protected:
            bool locked_getRowRange
            (
                int y,
                int xStart,
                int xEnd,
                std::vector<mdn::Digit>& out
            ) const;
        public:

            // Write a contiguous row back in one call. Interprets 0 as "setToZero".
            void setRowRange(int y, int xStart, const std::vector<mdn::Digit>& row);
            protected:
                void locked_setRowRange(int y, int xStart, const std::vector<mdn::Digit>& row);
            public:

            // Assembles the row at the given y index value, spanning the x bounds of full MDN
            std::vector<Digit> getRow(int y) const;
            protected: std::vector<Digit> locked_getRow(int y) const; public:

            // Assembles the row at the given y index value, spanning the x bounds of full MDN
            void getRow(int y, std::vector<Digit>& digits) const;
            protected: void locked_getRow(int y, std::vector<Digit>& digits) const; public:

            // Assembles the col at the given x index value, spanning the y bounds of full MDN
            std::vector<Digit> getCol(int x) const;
            protected: std::vector<Digit> locked_getCol(int x) const; public:

            // Assembles the col at the given x index value, spanning the y bounds of full MDN
            void getCol(int x, std::vector<Digit>& digits) const;
            protected: void locked_getCol(int x, std::vector<Digit>& digits) const; public:


        // *** Setters

            // Clears all digits in the MDN.
            void clear();
            protected: void locked_clear(); public:

            // Set the value at xy to zero
            //  Returns true if carryover status might change:
            //      * Value goes from zero to non-zero
            //      * Value changes sign
            bool setToZero(const Coord& xy);
            protected: bool locked_setToZero(const Coord& xy); public:

            // Set the value at coords to zero, returns subset containing those whose values changed
            CoordSet setToZero(const CoordSet& coords);
            protected: CoordSet locked_setToZero(const CoordSet& purgeSet); public:

            // Changes the value at xy
            //  Returns true if carryover status might change:
            //      * Value goes from zero to non-zero
            //      * Value changes sign
            bool setValue(const Coord& xy, Digit value);
            bool setValue(const Coord& xy, int value);
            bool setValue(const Coord& xy, long value);
            bool setValue(const Coord& xy, long long value);
            protected:
                bool locked_setValue(const Coord& xy, Digit value);
                bool locked_setValue(const Coord& xy, int value);
                bool locked_setValue(const Coord& xy, long value);
                bool locked_setValue(const Coord& xy, long long value);
            public:


        // *** Conversion / display

            // Converts the MDN to a human-readable string.
            std::string toString() const;
            protected: std::string locked_toString() const; public:

            // Converts the MDN to an array of strings, representing rows
            std::vector<std::string> toStringRows() const;
            protected: std::vector<std::string> locked_toStringRows() const; public:

            // Converts the MDN to an array of strings, representing columns
            std::vector<std::string> toStringCols() const;
            protected: std::vector<std::string> locked_toStringCols() const; public:


        // *** Transformations

            // Clears all addressing and bounds data, and rebuilds it
            void rebuildMetadata() const;
            protected: void locked_rebuildMetadata() const; public:

            // Returns true if m_bounds are both valid, finite numbers
            bool hasBounds() const;
            protected: bool locked_hasBounds() const; public:

            // Retuns bounds of non zero entries in m_raw
            const Rect& bounds() const;
            protected: const Rect& locked_getBounds() const; public:


        // *** Other functionality

            int getPrecision() const;
            protected: int locked_getPrecision() const; public:

            // Change the setting for m_precision, returns the number of dropped digits
            int setPrecision(int newMaxSpan);
            protected: int locked_setPrecision(int newMaxSpan); public:

            // Query the precision status of xy to ensure precision is not exceeded
            // Returns:
            //  * PrecisionStatus::Below  - above precision window
            //  * PrecisionStatus::Inside - within precision window
            //  * PrecisionStatus::Above  - below precision window
            PrecisionStatus checkPrecisionWindow(const Coord& xy) const;
            protected: PrecisionStatus locked_checkPrecisionWindow(const Coord& xy) const; public:

            // Register class as observer, inform when destroyed
            void registerObserver(MdnObserver* obs) const;
            protected: void locked_registerObserver(MdnObserver* obs) const;

            // Set the m_modified flag to trigger housekeeping with derived data when operations are
            // complete

            // Use when the data has changed, but operation may not yet be complete
            void internal_modified();

            // Use when the operation is complete, but the data may not have changed
            void internal_operationComplete();

            // Use when the data has changed AND the operation is complete, performs housekeeping:
            //  * clear derived data
            //  * increment m_event flag
            void internal_modifiedAndComplete();


protected:

    // *** Private Member Functions

        // Lock m_mutex for writeable reasons (unique_lock)
        WritableLock lockWriteable() const;

        // Lock m_mutex for read-only reasons (shareable_lock)
        ReadOnlyLock lockReadOnly() const;

        // Ensure the supplied argument 'that' is not the same as *this, throws if not
        void assertNotSelf(Mdn2dBase& that, const std::string& description) const;


        // *** Internal functions
        // All these functions require the mutex to be locked first before calling

            // Clears all addressing and bounds data
            virtual void internal_clearMetadata() const;

            // Sets value at xy without checking in range of base
            //  Returns true if carryover status might change:
            //      * Value goes from zero to non-zero
            //      * Value changes sign
            bool internal_setValueRaw(const Coord& xy, Digit value);

            // Add xy as a non-zero number position to the metadata
            void internal_insertAddress(const Coord& xy) const;

            // Checks if value is within +/- (m_base-1).  If not, throws or returns false.
            template <class Type>
            void internal_checkDigit(const Coord& xy, Type value) const {
                Digit baseDigit = m_config.baseDigit();

                if (Log_Showing_Debug4) {
                    Log_N_Debug4(
                        "Checking value " << static_cast<int>(value) << " against base "
                        << int(baseDigit)
                    );
                }
                if (value >= baseDigit || value <= -baseDigit) {
                    OutOfRange err(xy, static_cast<int>(value), baseDigit);
                    Log_N_Error(err.what());
                    throw err;
                }
            }

            // Purge any digits that exceed the precision window, return the number of purged digits
            int internal_purgeExcessDigits();

            // Update the m_bounds based on the current values
            void internal_updateBounds();

};

} // end namespace mdn
----- Mdn2dConfig.h -----
#pragma once

#include <cmath>
#include <sstream>

#include "Constants.h"
#include "Digit.h"
#include "Fraxis.h"
#include "GlobalConfig.h"
#include "Logger.h"
#include "MdnException.h"
#include "Mdn2dFramework.h"
#include "SignConvention.h"

namespace mdn {



// Contains all settings governing behaviour of an Mdn2d
class MDN_API Mdn2dConfig {

    // Calculate minimum fraction value to add to a digit, that it will appear as a non-zero
    // digit within m_precision
    static double static_calculateEpsilon(int precisionIn, int baseIn) {
        return pow((1.0 / baseIn), (precisionIn + 1));
    }

    // Pointer to framework governing class - what object holds this Mdn2d?
    static Mdn2dFramework* m_masterPtr;


public:

    static Mdn2dFramework& master() { return *m_masterPtr; }

    // Set master pointer to the given framework, posts warning if already set
    static void setMaster(Mdn2dFramework& framework);

    // Set master pointer to the given framework, overwrites previously existing setting silently
    static void resetMaster(Mdn2dFramework& framework);

    // If a message exists here, the associated Mdn2d is invalid for the reason contained in the
    // string.  Purpose of this is to prevent throwing during move ctor, so normal operation can be
    // optimised. Edge cases that invalidate the number show up here.
    mutable std::string m_invalidReason;

    // Numerical base, beyond which no digit's magnitude can go
    int m_base;

    // Convenience - base expressed as a Digit type
    Digit m_baseDigit;

    // Convenience - base expressed as a double type
    double m_baseDouble;

    // Maximum number of digits from lowest magnitude to maximum magnitude
    int m_precision;

    // Smallest value added to a digit that can cascade to a non-zero value within the precision
    //  window
    double m_epsilon;

    // Default sign convention for polymorphic numbers
    SignConvention m_signConvention;

    // Maximum number of outerloops allowed in attempting to reach polymorphic stability
    //  That is, all required carryovers are done, and polymorphic carryovers meet the default sign
    //  convention.  Use -1 for infinite loops allowed.
    int m_maxCarryoverIters;

    // Affects 1) fractional addition, 2) divide direction
    Fraxis m_defaultFraxis;


public:

    // Summon a purely default Mdn2dConfig object
    static Mdn2dConfig static_defaultConfig() { return Mdn2dConfig(); }


    // *** Constructors

    // Construct from parts, or null
    Mdn2dConfig(
        int baseIn=10,
        int maxSpanIn=16,
        SignConvention signConventionIn=SignConvention::Positive,
        int maxCarryoverItersIn = 20,
        Fraxis defaultFraxisIn=Fraxis::X
    );

    // Returns true if the number became invalid during a noexcept function
    bool valid() { return m_invalidReason.empty(); }

    // Returns false if the number became invalid during a noexcept function
    bool invalid() { return !valid(); }

    // Returns the reason this number became invalid during a noexcept function
    std::string invalidReason() { return m_invalidReason; }

    // Set this number to invalid - required to preserve nexcept optimisations, but also handle edge
    // cases that would have thrown
    void setInvalid(const std::string& reason) { m_invalidReason = reason; }

    // Return the base
    int base() const { return m_base; }

    // Return the base as a Digit type
    Digit baseDigit() const { return m_baseDigit; }

    // Return the base as a double type
    double baseDouble() const { return m_baseDouble; }

    // Return the numeric precision
    int precision() const { return m_precision; }

    // Change the precision, update downstream derived values
    void setPrecision(int precisionIn);

    // Return the derived epsilon value
    double epsilon() const { return m_epsilon; }

    SignConvention signConvention() const { return m_signConvention; }
    void setSignConvention(int newVal);
    void setSignConvention(std::string newName) {
        m_signConvention = NameToSignConvention(newName);
    }
    void setSignConvention(SignConvention signConventionIn) {
        m_signConvention = signConventionIn;
    }

    int maxCarryoverIters() { return m_maxCarryoverIters; }
    void setMaxCarryoverIters(int newVal) {
        m_maxCarryoverIters = newVal < 0 ? constants::intMax : newVal;
    }

    Fraxis defaultFraxis() const { return m_defaultFraxis; }
    void setDefaultFraxis(int newVal);
    void setDefaultFraxis(std::string newName) { m_defaultFraxis = NameToFraxis(newName); }
    void setDefaultFraxis(Fraxis fraxisIn) { m_defaultFraxis = fraxisIn; }

    // Returns true if all settings are valid, false if something failed
    bool checkConfig() const;

    // Throws if any setting is invalid
    void validateConfig() const;

    std::string to_string() const;

    // string format: (b:10, p:16, s:Positive, c:20, f:X)
    friend std::ostream& operator<<(std::ostream& os, const Mdn2dConfig& c) {
        return os
            << "("
                << "b:" << c.m_base
                << ", p:" << c.m_precision
                << ", e:" << c.m_epsilon
                << ", s:" << SignConventionToName(c.m_signConvention)
                << ", c:" << c.m_maxCarryoverIters
                << ", f:" << FraxisToName(c.m_defaultFraxis)
            << ")";
    }

    friend std::istream& operator>>(std::istream& is, Mdn2dConfig& c) {
        // TODO - problem, epsilon is derived from precision, should not be independent
        char lparen, letter, colon, comma, rparen;
        std::string fname;
        std::string sname;
        // (b:10, p:16, e:0.0000024, s:Positive, c:20, f:X)
        // Reading [(b:10]
        is >> lparen >> letter >> colon >> c.m_base;
        // Reading [, p:16]
        is >> comma >> letter >> colon >> c.m_precision;
        // Reading [, e:2.4e-6]
        is >> comma >> letter >> colon >> c.m_epsilon;
        // Reading [, s:Positive,]
        while (letter != ',') {
            is >> letter;
            sname += letter;
        }
        c.m_signConvention = NameToSignConvention(sname);
        //Reading [ c:20]
        is >> letter >> colon >> c.m_maxCarryoverIters;
        //Reading [, f:X])
        while (letter != ')') {
            is >> letter;
            fname += letter;
        }
        c.m_defaultFraxis = NameToFraxis(fname);
        return is;
    }

    // SignConvention m_signConvention;
    // int m_maxCarryoverIters;
    // Fraxis m_defaultFraxis;

        // From the perspective of an Mdn2d, look for compatibility.  The data that matters are:
        //  base, precision, sign convention
        bool operator==(const Mdn2dConfig& rhs) const;
        bool operator!=(const Mdn2dConfig& rhs) const;
};

} // end namespace mdn
----- Mdn2dConfigImpact.h -----
#pragma once

#include <vector>
#include <string>

#include "Logger.h"

// Sign enumeration

namespace mdn {

enum class Mdn2dConfigImpact {
    Unknown,
    NoImpact,
    AllDigitsCleared,
    PossibleDigitLoss,
    PossiblePolymorphism,
    PossibleDigitLossAndPolymorphism
};

const std::vector<std::string> Mdn2dConfigImpactNames(
    {
        "Unknown",
        "NoImpact",
        "AllDigitsCleared",
        "PossibleDigitLoss",
        "PossiblePolymorphism",
        "PossibleDigitLossAndPolymorphism"
    }
);

const std::vector<std::string> Mdn2dConfigImpactDescriptions(
    {
        "Unknown",
        "No impact",
        "Total loss - all digits will be cleared",
        "Possible loss of some digits",
        "Possible change in appearance but not value (polymorphism)",
        "Possible loss of some digits and a possible change in appearance"
    }
);

inline std::string Mdn2dConfigImpactToName(Mdn2dConfigImpact Mdn2dConfigImpact) {
    int fi = int(Mdn2dConfigImpact);
    return Mdn2dConfigImpactNames[fi];
}

inline std::string Mdn2dConfigImpactToDescription(Mdn2dConfigImpact Mdn2dConfigImpact) {
    int fi = int(Mdn2dConfigImpact);
    return Mdn2dConfigImpactDescriptions[fi];
}

inline Mdn2dConfigImpact NameToMdn2dConfigImpact(const std::string& name) {
    for (int i = 0; i < Mdn2dConfigImpactNames.size(); ++i) {
        if (Mdn2dConfigImpactNames[i] == name) {
            return static_cast<Mdn2dConfigImpact>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid Mdn2dConfigImpact type: " << name << " expecting:" << std::endl;
    if (Mdn2dConfigImpactNames.size()) {
        oss << Mdn2dConfigImpactNames[0];
    }
    for (auto iter = Mdn2dConfigImpactNames.cbegin() + 1; iter != Mdn2dConfigImpactNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
----- Mdn2dFramework.h -----
#pragma once

#include "../library/GlobalConfig.h"

// Interface class for Mdn2d number framework class, for Mdn2d to talk upwards to the framework
// within which they exist

namespace mdn {

class MDN_API Mdn2dFramework {

public:

    // Returns the framework's derived class type name as a string
    virtual std::string className() const {
        return "Mdn2dFramework";
    }

    // Returns the framework's 'name', used in error messaging
    virtual std::string name() const {
        return "DummyFramework";
    }

    // Returns the framework's 'name', used in error messaging
    virtual void setName(const std::string& nameIn) {
        // Do nothing
    }

    // Returns true if an Mdn2d exists with the given name, false otherwise
    virtual bool mdnNameExists(const std::string& nameIn) const {
        // default allows name collisions
        return false;
    }

    // Gives framework final say over name changes - given a desired Mdn name change from origName
    //  to newName, returns the allowed name
    virtual std::string requestMdnNameChange(
        const std::string& origName,
        const std::string& newName
    ) {
        // Default always allows the name change
        return newName;
    }

};

static Mdn2dFramework DummyFramework;

} // end namespace mdn----- Mdn2dRules.h -----
#pragma once

#include "Carryover.h"
#include "GlobalConfig.h"
#include "Mdn2dBase.h"

namespace mdn {

// Basic rules layer of 2d multi dimensional numbers, establishes:
//  * carryover rules
//  * polymorphism
//  * equality operator, now available with polymorphism
//  * digit shifting
//  * transposing, because why not?
class MDN_API Mdn2dRules : public Mdn2dBase {

protected:

    // Polymorphic nodes and the last time it was calculated
    mutable CoordSet m_polymorphicNodes;
    mutable long long m_polymorphicNodes_event;


public:

    // Check for the type of carryover, given the pivot digit and x and y axial digits
    static Carryover static_checkCarryover(Digit p, Digit x, Digit y, Digit base);


    // *** Constructors

        // Construct null
        Mdn2dRules(std::string nameIn="");

        // Construct from a configuration
        Mdn2dRules(Mdn2dConfig config, std::string nameIn="");


        // *** Rule of five

            // Copy constructor
            Mdn2dRules(const Mdn2dRules& other, std::string nameIn="");

            // Assignment operator
            Mdn2dRules& operator=(const Mdn2dRules& other);

            // Move operator
            Mdn2dRules(Mdn2dRules&& other, std::string nameIn="") noexcept;

            // Move assignment operator
            Mdn2dRules& operator=(Mdn2dRules&& other) noexcept;

            // Virtual destructor
            virtual ~Mdn2dRules() {}


    // *** Member Functions

        // *** Config

            // Change the config - can lead to any of the Mdn2dConfigImpact effects
            // Overriding virtual fn from base for access to carryover functionality
            protected: void locked_setConfig(Mdn2dConfig newConfig) override; public:


        // *** Transformations

            // Return the Carryover type (enumareation) of xy
            Carryover checkCarryover(const Coord& xy) const;
            protected: Carryover locked_checkCarryover(const Coord& xy) const; public:

            // Perform a manual carry-over at coordinate (x, y), returns affected coords as a set
            CoordSet carryover(const Coord& xy);
            protected:
                // Internal function takes arg 'carry' to be added to root of carryover, xy
                CoordSet locked_carryover(const Coord& xy, int carry = 0);
            public:

            // Given set of suspicious coords, check if any need carryovers, and if so, do them
            //  Returns set of coordinates that actually have changed
            CoordSet carryoverCleanup(const CoordSet& coords);
            protected: CoordSet locked_carryoverCleanup(const CoordSet& coords); public:

            // Given all the non-zero coords, check if any need carryovers, and if so, do them
            //  Returns set of coordinates that actually have changed
            CoordSet carryoverCleanupAll();
            protected: CoordSet locked_carryoverCleanupAll(); public:

            // General shift interface
            void shift(int xDigits, int yDigits);
            void shift(const Coord& xy);
            protected:
                void locked_shift(const Coord& xy);
                void locked_shift(int xDigits, int yDigits);
            public:

            // Shift all digits in a direction (R=+X, L=-X, U=+Y, D=-Y)
            void shiftRight(int nDigits);
            protected: void locked_shiftRight(int nDigits); public:
            void shiftLeft(int nDigits);
            protected: void locked_shiftLeft(int nDigits); public:
            void shiftUp(int nDigits);
            protected: void locked_shiftUp(int nDigits); public:
            void shiftDown(int nDigits);
            protected: void locked_shiftDown(int nDigits); public:

            // Swap X and Y
            void transpose();
            protected: void locked_transpose(); public:

            // Returns polymorphic nodes, demand driven
            const CoordSet& getPolymorphicNodes() const;
            protected: const CoordSet& locked_getPolymorphicNodes() const; public:


protected:

    // *** Protected Member Functions

        // Find all the 'Optional' carryovers, create m_polymorphicNodes data
        void internal_polymorphicScan() const;

        // Perform a blind single carryover at xy without any checks
        void internal_oneCarryover(const Coord& xy);

        // Perform a carryover during math operations (xy magnitude must exceed base)
        void internal_ncarryover(const Coord& xy);

        // Clear all derived data, including polymorphism-related data
        virtual void internal_clearMetadata() const override;

};

} // end namespace mdn

----- MdnException.h -----
// This header defines error-handling strategy for MDN operations
#pragma once

#include <stdexcept>
#include <string>
#include "Coord.h"

namespace mdn {

// Base class for all MDN-related runtime errors
class MDN_API MdnException : public std::runtime_error {
public:
    explicit MdnException(const std::string& msg) : std::runtime_error(msg) {}
};

// Attempt to carry over from a zero digit
class MDN_API FailedAssertion : public MdnException {
public:
    FailedAssertion(const std::string& description)
        : MdnException(description) {}
};

// Attempt to carry over from a zero digit
class MDN_API InvalidCarryOver : public MdnException {
public:
    InvalidCarryOver(const Coord& xy)
        : MdnException("Coordinate " + xy.to_string() + ": invalid carry over at zero digit.") {}
};

// Attempt to assign out-of-range value to digit
class MDN_API OutOfRange : public MdnException {
public:
    OutOfRange(const Coord& xy, int value, int base)
        : MdnException(
            "Coordinate " + xy.to_string() + ": out-of-range value " + std::to_string(value)
            + ", expecting ±" + std::to_string(base-1)
        ) {}
};

// Attempt to assign out-of-range value to digit
class MDN_API ZeroEncountered : public MdnException {
public:
    ZeroEncountered(const Coord& xy)
        : MdnException(
            "Coordinate " + xy.to_string() + ": has recorded value of 0.  Sparse storage should " +
            "never contain a zero."
        ) {}
};

// MetaData is out of sync or invalid
class MDN_API MetaDataInvalid : public MdnException {
public:
    MetaDataInvalid(const std::string& description)
        : MdnException("MDN MetaData invalid: " + description) {};
};

// Argument cannot be self
class MDN_API IllegalSelfReference : public MdnException {
public:
    IllegalSelfReference(const std::string& description)
        : MdnException("Supplied argument must not be the exact same object: " + description) {};
};

// Bases must match
class MDN_API BaseMismatch : public MdnException {
public:
    BaseMismatch(int baseA, int baseB)
        : MdnException(
            "Bases of interacting Mdn2d objects must match: (" + std::to_string(baseA) +
            " != " + std::to_string(baseB) + ")"
        ) {};
};

// This operation breaks the governing laws of MDNs
class MDN_API IllegalOperation : public MdnException {
public:
    IllegalOperation(const std::string& description)
        : MdnException("Illegal operation: " + description) {};
};

// The objects or data have encountered an invalid state
class MDN_API InvalidState: public MdnException {
public:
    InvalidState(const std::string& description)
        : MdnException("Invalid state detected: " + description) {};
};

// Division by zero is about to occur
class MDN_API DivideByZero : public MdnException {
public:
    DivideByZero()
        : MdnException("Division by zero") {};
};

// Argument is not valid
class MDN_API InvalidArgument : public MdnException {
public:
    InvalidArgument(const std::string& description)
        : MdnException("Invalid argument detected: " + description) {};
};

// Cannot perform the operation for the given described reason
class MDN_API InvalidOperation : public MdnException {
public:
    InvalidOperation(const std::string& description)
        : MdnException("Invalid operation detected: " + description) {};
};


} // namespace mdn
----- MdnObserver.h -----
#pragma once

#include "Logger.h"

namespace mdn {

class Mdn2d;

// Hold a reference to an Mdn2d, and observe it for changes to stay up-to-date
class MdnObserver {

protected:

    // The object I am observing
    mutable Mdn2d* m_ref = nullptr;

    int m_instance = -1;


public:

    // Return instance of this observer
    int instance() const {
        return m_instance;
    }

    // Set the instance of this observer
    int setInstance(int instance) {
        #ifdef MDN_DEBUG
            if (m_instance >= 0) {
                Log_Warn("Setting MdnObserver instance when it is already set.");
            }
        #endif
        m_instance = instance;
    }

    // Set the reference Mdn2d
    virtual void set(Mdn2d* ref) {
        m_ref = ref;
    }

    // Return the reference Mdn2d
    virtual Mdn2d* get() const {
        return m_ref;
    }

    // The observed object has been modified
    virtual void modified() const {}

    // The observed object is being reallocated to a new address
    virtual void reallocating(Mdn2d* newRef) {
        m_ref = newRef;
    }

    // The observed object is being destroyed
    virtual void farewell() {
        m_ref = nullptr;
    }

};

} // End namespace mdn
----- NamedEnum.h -----
#pragma once

#include <vector>
#include <string>

#include "Logger.h"

// A few general tools to convert templated Enum into its associated name, and visa-versa
// Not actually used yet.

namespace mdn {

template<class Enum>
std::string EnumToName(Enum e, const std::vector<std::string>& names) {
    int ei = int(e);
    return names[ei];
}

template<class Enum>
Enum NameToEnum(const std::string& name, std::vector<std::string>& names) {
    for (int i = 0; i < names.size(); ++i) {
        if (names[i] == name) {
            return static_cast<Enum>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid Enum type: " << name << " expecting:" << std::endl;
    if (names.size()) {
        oss << names[0];
    }
    for (auto iter = names.cbegin() + 1; iter != names.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
----- PolymorphicTree.h -----
#pragma once

/*
See also Carryover.h

This file is me trying to figure out ways to fully capture all polymorphic states of an MDN.
The current library does not use any of this.  Consider this just a collection of notes.

Polymorphic data structure (pmds)
We start with an Mdn in a valid state.  All required c/o's have been performed.  The polymorphic
data is probably going to be a tree.  Top level nodes are all optional c/o's.

[0,0] [2,0] [0,4] [1,5] ...
  |     |     |     |

Below each one are downstream c/o's that may be altered / awakened / removed, for example:

[0,0]
    * [0,1] - from Optional --> Invalid
    * [1,0] - from Invalid --> Required

Any newly created 'Required' c/o's must be actuated during the construction of the pmds.

[0,0]
    * [0,1] -Optional
    * [1,0] +Required --> [2,0] no change (invalid in this example)
                      \-> [1,1] +Optional

Some 'optional' nodes can appear more than once.

Static representation
    Coord
        Toggles [c0, c1, ...]


Node:

    Coord (x, y)
        The coordinates of the polymorphic carryover / optional carryover
    Downstream  [c0, c1, ...]
        The downstream required c/o's
    Activates [c0, c1, ...]
        What carryovers it activates (original coord and downstream)
    Deactivates [c0, c1, ...]
        What carryovers it deactivates

    Map (Coord)
        How to get to carryover this digit, if at all
    Activates [[c0, c1, ...],[c0, c1, ...]]
        Each list of coords is a list of all necessary c/o's needed before reaching Coord
        Multiple lists because sometimes more than one route leads to the same c/o

    Straight up, honest tree
    C0-,-C1-C3
       '-C2
*/

#include <unordered_map>
#include <utility>

#include "Coord.h"

namespace mdn {

struct PolymorphicTree {
    std::unordered_map<Coord, PolymorphicTreeNode> m_tree;
    std::unordered_set<Coord> m_currentIndex;
    std::vector<PolymorphicTreeNode> m_breadcrumbs;

    void clear() {
        m_tree.clear();
        m_currentIndex.clear();
        m_breadcrumbs.clear();
    }
};

struct PolymorphicTreeNode {
    Coord m_root;
    std::vector<PolymorphicTreeNode> m_deactivations;
    std::vector<PolymorphicTreeNode> m_activations;
};



enum class Carryover {
    Invalid,            // Valid MDN --> carryover --> Invalid MDN
    OptionalPositive,   // Valid MDN --> carryover --> Valid MDN, current value positive
    OptionalNegative,   // Valid MDN --> carryover --> Valid MDN, current value negative
    Required            // Invalid MDN --> carryover --> Valid MDN
};

} // end namespace----- PrecisionStatus.h -----
#pragma once

// Precision status
//  Is this coordinate within the precision window; if not is it above or below?

#include <vector>
#include <string>

#include "Logger.h"

namespace mdn {

enum class PrecisionStatus {
    Below,
    Inside,
    Above
};

const std::vector<std::string> PrecisionStatusNames(
    {
        "Below",
        "Inside",
        "Above"
    }
);

inline std::string PrecisionStatusToName(PrecisionStatus precisionStatus) {
    int fi = int(precisionStatus);
    // #ifdef MDN_DEBUG
    //     if (fi < 0 || fi >= PrecisionStatusNames.size())
    //         Logger::instance().error("PrecisionStatus out of range: " + std::to_string(fi));
    // #endif
    return PrecisionStatusNames[fi];
}

inline PrecisionStatus NameToPrecisionStatus(const std::string& name) {
    for (int i = 0; i < PrecisionStatusNames.size(); ++i) {
        if (PrecisionStatusNames[i] == name) {
            return static_cast<PrecisionStatus>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid PrecisionStatus type: " << name << " expecting:" << std::endl;
    if (PrecisionStatusNames.size()) {
        oss << PrecisionStatusNames[0];
    }
    for (auto iter = PrecisionStatusNames.cbegin() + 1; iter != PrecisionStatusNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
----- Rect.h -----
#pragma once

#include "Coord.h"
#include "GlobalConfig.h"
#include <vector>
#include <algorithm>
#include <limits>

namespace mdn {

class MDN_API Rect {

public:

    // *** Static member functions

    // Returns an invalid rectangle, plays nice with growToInclude
    static Rect GetInvalid() {
        return Rect(
            Coord(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()),
            Coord(std::numeric_limits<int>::min(), std::numeric_limits<int>::min())
        );
    }

    // Return intersection of two rects. If they don't intersect, returns Rect::GetInvalid()
    inline static Rect Intersection(const Rect& a, const Rect& b) {
        Coord minPt(
            std::max(a.min().x(), b.min().x()),
            std::max(a.min().y(), b.min().y())
        );
        Coord maxPt(
            std::min(a.max().x(), b.max().x()),
            std::min(a.max().y(), b.max().y())
        );

        Rect result(minPt, maxPt);
        return result.isValid() ? result : Rect::GetInvalid();
    }

    // Return the smallest rect that contains both input rects
    inline static Rect UnionOf(const Rect& a, const Rect& b) {
        if (!a.isValid()) return b;
        if (!b.isValid()) return a;

        Coord minPt(
            std::min(a.min().x(), b.min().x()),
            std::min(a.min().y(), b.min().y())
        );
        Coord maxPt(
            std::max(a.max().x(), b.max().x()),
            std::max(a.max().y(), b.max().y())
        );

        return Rect(minPt, maxPt);
    }

    // Check if rects overlap (intersection is non-empty)
    inline static bool Overlaps(const Rect& a, const Rect& b) {
        return Intersection(a, b).isValid();
    }

    // Check if two rects are adjacent but not overlapping
    inline static bool AreAdjacent(const Rect& a, const Rect& b) {
        if (Overlaps(a, b)) return false;

        // One of the edges must touch
        bool horizontalTouch =
            (a.right() + 1 == b.left() || b.right() + 1 == a.left()) &&
            !(a.top() < b.bottom() || b.top() < a.bottom());

        bool verticalTouch =
            (a.top() + 1 == b.bottom() || b.top() + 1 == a.bottom()) &&
            !(a.right() < b.left() || b.right() < a.left());

        return horizontalTouch || verticalTouch;
    }


    // *** Constructors

    // Construct null - retuns an invalid rectangle
    Rect() : Rect(GetInvalid()) {}

    // Construct from components
    //  fixOrdering - when true, corrects inputs where individual components of min and max might
    //      need to be swapped. Useful when source coords are unreliable. See fixOrdering function.
    Rect(Coord min, Coord max, bool fixOrdering = false)
        : m_min(min), m_max(max)
    {
        if (fixOrdering) {
            this->fixOrdering();
        }
    }

    // Construct from elemental components
    //  fixOrdering - when true, corrects inputs where individual components of min and max might
    //      need to be swapped. Useful when source coords are unreliable. See fixOrdering function.
    Rect(int xmin, int ymin, int xmax, int ymax, bool fixOrdering = false)
        : m_min(xmin, ymin), m_max(xmax, ymax)
    {
        if (fixOrdering) {
            this->fixOrdering();
        }
    }


    // *** Rule of five
    //  Compliant by default

        Rect(const Rect&) = default;
        Rect(Rect&&) = default;
        Rect& operator=(const Rect&) = default;
        Rect& operator=(Rect&&) = default;
        ~Rect() = default;


    // *** Public member functions

    // Returns true if this Rect is valid, i.e. min and max make sense
    bool isValid() const {
        return m_min.x() <= m_max.x() && m_min.y() <= m_max.y();
    }

    // Returns true if this Rect is invalid, i.e. min and max do not make sense
    bool isInvalid() const {
        return !isValid();
    }

    // Treats each component independently, ensures min and max are in the correct variable
    void fixOrdering() {
        if (m_min.x() > m_max.x()) std::swap(m_min.x(), m_max.x());
        if (m_min.y() > m_max.y()) std::swap(m_min.y(), m_max.y());
    }

    Coord min() const { return m_min; }
    Coord max() const { return m_max; }

    int left()   const { return m_min.x(); }
    int right()  const { return m_max.x(); }
    int bottom() const { return m_min.y(); }
    int top()    const { return m_max.y(); }

    int width()  const { return m_max.x() - m_min.x() + 1; }
    int height() const { return m_max.y() - m_min.y() + 1; }

    // A mathematical vector from min to max, gives (nColumns-1, nRows-1)
    Coord delta() const { return m_max - m_min; }

    // Count for number of elements along each dimension, gives (nColumns, nRows)
    Coord gridSize() const { return Coord(width(), height()); }

    // Returns the total number of integer Coordinates covered by this object, does not count non-
    //  zeros.
    int size() const {
        return isValid() ? width() * height() : 0;
    }

    bool isSingleCoord() const {
        return isValid() && m_min == m_max;
    }

    bool isMultiCoord() const {
        return isValid() && m_min != m_max;
    }

    bool contains(const Coord& c) const {
        return c.x() >= m_min.x() && c.x() <= m_max.x()
            && c.y() >= m_min.y() && c.y() <= m_max.y();
    }

    void clear() {
        *this = Rect::GetInvalid();
    }

    void set(Coord min, Coord max, bool fixOrdering = false) {
        m_min = min;
        m_max = max;
        if (fixOrdering) {
            this->fixOrdering();
        }
    }
    void set(int xmin, int ymin, int xmax, int ymax, bool fixOrdering = false) {
        m_min.set(xmin, ymin);
        m_max.set(xmax, ymax);
        if (fixOrdering) {
            this->fixOrdering();
        }
    }
    void setMin(int x, int y, bool fixOrdering = false) {
        m_min.set(x, y);
        if (fixOrdering) {
            this->fixOrdering();
        }
    }
    void setMax(int x, int y, bool fixOrdering = false) {
        m_max.set(x, y);
        if (fixOrdering) {
            this->fixOrdering();
        }
    }

    void translate(int dx, int dy) {
        m_min.translate(dx, dy);
        m_max.translate(dx, dy);
    }

    Rect translated(int dx, int dy) const {
        return Rect(m_min.translated(dx, dy), m_max.translated(dx, dy));
    }

    void growToInclude(const Coord& c) {
        if (!isValid()) {
            m_min = m_max = c;
            return;
        }
        if (c.x() < m_min.x()) m_min.x() = c.x();
        if (c.x() > m_max.x()) m_max.x() = c.x();
        if (c.y() < m_min.y()) m_min.y() = c.y();
        if (c.y() > m_max.y()) m_max.y() = c.y();
    }

    std::vector<Coord> toCoordVector() const {
        std::vector<Coord> coords;
        if (!isValid()) return coords;
        for (int y = bottom(); y <= top(); ++y) {
            for (int x = left(); x <= right(); ++x) {
                coords.emplace_back(x, y);
            }
        }
        return coords;
    }

    friend std::ostream& operator<<(std::ostream& os, const Rect& r) {
        if (!r.isValid()) {
            os << "[Empty]";
        } else {
            os << "[" << r.min() << " -> " << r.max() << "]";
        }
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Rect& r) {
        char ch;
        is >> ch;

        if (ch != '[') {
            is.setstate(std::ios::failbit);
            return is;
        }

        // Peek ahead to see if it's "Empty]"
        std::string word;
        is >> word;

        if (word == "Empty]") {
            r = Rect::GetInvalid();
            return is;
        }

        // Back up stream to parse normally
        is.putback(word.back());
        for (int i = static_cast<int>(word.size()) - 2; i >= 0; --i)
            is.putback(word[i]);

        Coord min, max;
        is >> min;

        is >> ch; // read '-'
        if (ch != '-') {
            is.setstate(std::ios::failbit);
            return is;
        }

        is >> ch; // expect '>'
        if (ch != '>') {
            is.setstate(std::ios::failbit);
            return is;
        }

        is >> max;

        is >> ch; // read ']'
        if (ch != ']') {
            is.setstate(std::ios::failbit);
            return is;
        }

        r = Rect(min, max);
        return is;
    }


private:

    Coord m_min;
    Coord m_max;
};

} // end namespace mdn
----- SignConvention.h -----
#pragma once

#include <vector>
#include <string>

#include "Logger.h"

// Sign enumeration

namespace mdn {

enum class SignConvention {
    Invalid,
    Default,
    Positive,
    Negative
};

const std::vector<std::string> SignConventionNames(
    {
        "Invalid",
        "Default",  // Or, do not change
        "Positive",
        "Negative"
    }
);

inline std::string SignConventionToName(SignConvention SignConvention) {
    int fi = int(SignConvention);
    // #ifdef MDN_DEBUG
    //     if (fi < 0 || fi >= SignConventionNames.size())
    //         Logger::instance().error("SignConvention out of range: " + std::to_string(fi));
    // #endif
    return SignConventionNames[fi];
}

inline SignConvention NameToSignConvention(const std::string& name) {
    for (int i = 0; i < SignConventionNames.size(); ++i) {
        if (SignConventionNames[i] == name) {
            return static_cast<SignConvention>(i);
        }
    }
    std::ostringstream oss;
    oss << "Invalid SignConvention type: " << name << " expecting:" << std::endl;
    if (SignConventionNames.size()) {
        oss << SignConventionNames[0];
    }
    for (auto iter = SignConventionNames.cbegin() + 1; iter != SignConventionNames.cend(); ++iter) {
        oss << ", " << name;
    }
    throw std::invalid_argument(oss.str());
}


} // end namespace mdn
----- Tools.h -----
#pragma once

#include <cstdint>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Constants.h"
#include "Digit.h"
#include "GlobalConfig.h"

namespace mdn {

class MDN_API Tools {

public:

    static const std::vector<char> m_digToAlpha;
    static const std::string m_boxArt_h; // ─
    static const std::string m_boxArt_v; // │
    static const std::string m_boxArt_x; // ┼

    template <class T>
    struct is_byte_like
    : std::bool_constant<
            std::is_integral_v<std::remove_cv_t<T>> &&
            sizeof(std::remove_cv_t<T>) == 1> {};
    template <class T>
    inline static constexpr bool is_byte_like_v = is_byte_like<T>::value;


    // Convert a vector of anything to a string delimiter
    template<typename T>
    static std::string vectorToString(
        const std::vector<T>& array, const std::string& delimiter, bool reverse
    ) {
        if (array.empty()) {
            return "";
        }

        std::ostringstream oss;
        if (reverse) {
            size_t lastI = array.size()-1;
            if constexpr (is_byte_like_v<T>)
                out << static_cast<int>(array[lastI]);
            else
                out << array[lastI];
            for (size_t i = lastI-1; i >= 0; --i) {
                if constexpr (is_byte_like_v<T>)
                    out << delimiter << static_cast<int>(array[i]);
                else
                    out << delimiter << array[i];
            }
        } else {
            if constexpr (is_byte_like_v<T>)
                out << static_cast<int>(array[0]);
            else
                out << array[0];
            for (size_t i = 1; i < array.size(); ++i) {
                oss << delimiter << array[i];
                if constexpr (is_byte_like_v<T>)
                    out << delimiter <<static_cast<int>(array[i]);
                else
                    out << delimiter << array[i];
            }
        }
        return oss.str();
    }

    // Optional overload for char delimiter
    template<typename T>
    static std::string vectorToString(const std::vector<T>& array, char delimiter, bool reverse) {
        return vectorToString(array, std::string(1, delimiter), reverse);
    }

    template<typename S, typename T>
    static std::string pairToString(const std::pair<S, T>& pair, const std::string& delimiter) {
        std::ostringstream oss;
        oss << pair.first << delimiter << pair.second;
        return oss.str();
    }

    // Optional overload for char delimiter
    template<typename S, typename T>
    static std::string pairToString(const std::pair<S, T>& pair, char delimiter) {
        return pairToString(pair, std::string(1, delimiter));
    }

    // Convert a set of anything to a string delimiter
    template<typename T>
    static std::string setToString(const std::unordered_set<T>& set, const std::string& delimiter) {
        if (set.empty()) return "";

        std::ostringstream oss;
        bool first = true;
        for (T elem : set) {
            if (first) {
                first = false;
                oss << elem;
            } else {
                oss << delimiter << elem;
            }
        }
        return oss.str();
    }

    // Optional overload for char delimiter
    template<typename T>
    static std::string setToString(const std::unordered_set<T>& set, char delimiter) {
        return setToString(set, std::string(1, delimiter));
    }

    // Converts a value between -32 and 32 to -V,..,-C,-B,-A,-9,-8, .. ,-1,0,1,...,8,9,A,B,...,V
    static std::string digitToAlpha(
        Digit value, std::string pos=" ", std::string neg=m_boxArt_h
    );
    static std::string digitToAlpha(
        int value, std::string pos=" ", std::string neg=m_boxArt_h
    );
    static std::string digitToAlpha(
        long value, std::string pos=" ", std::string neg=m_boxArt_h
    );
    static std::string digitToAlpha(
        long long value, std::string pos=" ", std::string neg=m_boxArt_h
    );

    // Convert a std::vector<Digit> to a string
    static std::string digitArrayToString(
        const std::vector<Digit>& array,
        char delim=',',
        char open='(',
        char close=')'
    );

    // Ensure div is not too close to zero
    static void stabilise(float& div);
    static void stabilise(double& div);

    // Get filename from full path
    static std::string removePath(const char* fullpath);

};

} // namespace mdn
