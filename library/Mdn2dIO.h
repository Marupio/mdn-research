#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include "GlobalConfig.h"
#include "Mdn2dBase.h"
#include "Rect.h"
#include "TextOptions.h"
#include "Tools.h"

namespace mdn {

class MDN_API Mdn2dIO {
public:
    // -------- Text -> strings --------
    static std::vector<std::string> toStringRows(
        const Mdn2dBase& src,
        const TextWriteOptions& opt = TextWriteOptions::DefaultPretty()
    );

    static std::vector<std::string> toStringCols(
        const Mdn2dBase& src,
        const TextWriteOptions& opt = TextWriteOptions::DefaultUtility()
    );

    // -------- Text -> streams (convenience wrappers) --------
    static void saveTextPretty(
        const Mdn2dBase& src,
        std::ostream& os,
        const TextWriteOptions& opt = TextWriteOptions::DefaultPretty()
    );

    static void saveTextUtility(
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

    // -------- Binary I/O --------
    static void saveBinary(
        const Mdn2dBase& src,
        std::ostream& os
    );

    static void loadBinary(
        std::istream& is,
        Mdn2dBase& dst
    );

    // -------- Dispatcher --------
    // Sniffs stream; calls loadBinary() if magic matches, else loadText().
    static TextReadSummary load(
        std::istream& is,
        Mdn2dBase& dst
    );
};

// Stream operators (ASCII text). No trailing newline added.
inline std::ostream& operator<<(std::ostream& os, const Mdn2dBase& mdn) {
    Log_Debug3_H(mdn.getName());
    Mdn2dIO::saveTextPretty(mdn, os, TextWriteOptions::DefaultPretty());
    Log_Debug3_T("");
    return os;
}

inline std::istream& operator>>(std::istream& is, Mdn2dBase& mdn) {
    Log_Debug3_H("");
    Mdn2dIO::load(is, mdn);
    Log_Debug3_T(mdn.getName());
    return is;
}

} // namespace mdn
