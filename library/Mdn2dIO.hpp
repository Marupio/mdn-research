#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include "GlobalConfig.hpp"
#include "Mdn2dBase.hpp"
#include "MdnException.hpp"
#include "Rect.hpp"
#include "TextOptions.hpp"
#include "Tools.hpp"

namespace mdn {

class MDN_API Mdn2dIO {

    // *** Private static functions

    // Outputs 'text format' the src name. bounds and config
    static void internal_saveTextHeader(const Mdn2dBase& src, std::ostream& os);

    // Load header - name line
    static bool internal_parseNameLine(const std::string& line, std::string& outName);

    // Load header - bounds line
    static bool internal_parseBoundsLine(const std::string& line, int& x0, int& y0, int& x1, int& y1, bool& empty);

    // Load header - config line
    static bool internal_parseConfigLine(
        const std::string& line, int& base, int& prec, SignConvention& sign
    );

    static char internal_identifyDelim(const std::string& ascii);


public:
    // -------- Text -> strings --------
    static std::vector<std::string> toStringRows(
        const Mdn2dBase& src,
        const TextWriteOptions& opt = TextWriteOptions::DefaultPretty()
    );
    static std::vector<std::string> locked_toStringRows(
        const Mdn2dBase& src,
        const TextWriteOptions& opt = TextWriteOptions::DefaultPretty()
    );

    static std::vector<std::string> toStringCols(
        const Mdn2dBase& src,
        const TextWriteOptions& opt = TextWriteOptions::DefaultUtility()
    );
    static std::vector<std::string> locked_toStringCols(
        const Mdn2dBase& src,
        const TextWriteOptions& opt = TextWriteOptions::DefaultUtility()
    );

    // -------- Text -> streams (convenience wrappers) --------
    static void saveTextPretty(
        const Mdn2dBase& src,
        std::ostream& os,
        const TextWriteOptions& opt = TextWriteOptions::DefaultPretty()
    );
    static void locked_saveTextPretty(
        const Mdn2dBase& src,
        std::ostream& os,
        const TextWriteOptions& opt = TextWriteOptions::DefaultPretty()
    );

    static void saveTextUtility(
        const Mdn2dBase& src,
        std::ostream& os,
        const TextWriteOptions& opt = TextWriteOptions::DefaultUtility()
    );
    static void locked_saveTextUtility(
        const Mdn2dBase& src,
        std::ostream& os,
        const TextWriteOptions& opt = TextWriteOptions::DefaultUtility()
    );

    // Auto-detects pretty/utility by content; clears and writes into dst
    // Throws std::runtime_error on hard parse errors.
    static TextReadSummary loadText(
        std::istream& is,
        Mdn2dBase& dst
    );
    static TextReadSummary locked_loadText(
        std::istream& is,
        Mdn2dBase& dst
    );

    // -------- Binary I/O --------
    static void saveBinary(
        const Mdn2dBase& src,
        std::ostream& os
    );
    static void locked_saveBinary(
        const Mdn2dBase& src,
        std::ostream& os
    );

    static void loadBinary(
        std::istream& is,
        Mdn2dBase& dst
    );
    static void locked_loadBinary(
        std::istream& is,
        Mdn2dBase& dst
    );

    // -------- Dispatcher --------
    // Sniffs stream; calls loadBinary() if magic matches, else loadText().
    static TextReadSummary load(
        std::istream& is,
        Mdn2dBase& dst
    );
    static TextReadSummary locked_load(
        std::istream& is,
        Mdn2dBase& dst
    );
};

// Stream operators (ASCII text). No trailing newline added.
inline std::ostream& operator<<(std::ostream& os, const Mdn2dBase& mdn) {
    Log_Debug3_H(mdn.name());
    Mdn2dIO::saveTextPretty(mdn, os, TextWriteOptions::DefaultPretty());
    Log_Debug3_T("");
    return os;
}

inline std::istream& operator>>(std::istream& is, Mdn2dBase& mdn) {
    Log_Debug3_H("");
    Mdn2dIO::load(is, mdn);
    Log_Debug3_T(mdn.name());
    return is;
}

} // namespace mdn
