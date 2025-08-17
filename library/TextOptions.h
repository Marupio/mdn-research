#pragma once

#include <string>

#include "GlobalConfig.h"
#include "MdnException.h"
#include "Rect.h"

namespace mdn {

// Text axes rendering style
enum class AxesOutput { None, BoxArt, Simple };

// Delimiter choice for utility text
enum class CommaTabSpace { Comma, Tab, Space };
inline std::string toString(CommaTabSpace delim) {
    switch (delim) {
        case CommaTabSpace::Comma:
            return ",";
        case CommaTabSpace::Tab:
            return "\t";
        case CommaTabSpace::Space:
            return " ";
        default:
            std::ostringstream oss;
            oss << "Unrecognized CommaTabSpace: '" << static_cast<int>(delim)
                << "', expecting 'Comma', 'Tab', 'Space'";
            throw InvalidState(oss.str());
    }
}


// Options for writing text
struct MDN_API TextWriteOptions {

    // When axes != None, draw axes crossing at origin (0,0)
    AxesOutput axes = AxesOutput::BoxArt;

    // When true, emit 0-9, a..z for |digit| < 36; otherwise full integers
    bool alphanumeric = true;

    // When true, pretty mode uses wide box-draw for negatives inside grid
    bool wideNegatives = true;

    // CSV/TSV/etc for utility mode
    CommaTabSpace delim = CommaTabSpace::Space;

    // Optional clamp window; if invalid → use bounds()
    Rect window = Rect::GetInvalid();

    // Ready-made presets
    static TextWriteOptions DefaultPretty();
    static TextWriteOptions DefaultUtility(CommaTabSpace d = CommaTabSpace::Space);

};

// Result of reading text (pretty/utility) back into an MDN
struct MDN_API TextReadSummary {

    // bottom-left inclusive to top-right inclusive
    Rect parsedRect = Rect::GetInvalid();

    // columns
    int width = 0;

    // rows
    int height = 0;

    friend std::ostream& operator<<(std::ostream& os, const TextReadSummary& t) {
        return os << "{" << t.parsedRect << ", (" << t.width << ", " << t.height << ")}";
    }
};


} // end namespace mdn
