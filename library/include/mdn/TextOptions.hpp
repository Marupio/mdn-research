#pragma once

#include <string>

#include <mdn/GlobalConfig.hpp>
#include <mdn/MdnException.hpp>
#include <mdn/Rect.hpp>

namespace mdn {

// Text axes rendering style
enum class AxesOutput { None, BoxArt, Simple };
inline std::string toString(AxesOutput ao) {
    switch (ao) {
        case AxesOutput::BoxArt: return "BoxArt";
        case AxesOutput::Simple: return "Simple";
        default: return "None";
    }
}

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

enum class RowOrder {
    BottomToTop,   // legacy/engine-centric: y0..y1
    TopToBottom    // human/top-first: y1..y0
};
inline std::string toString(RowOrder ro) {
    switch (ro) {
        case RowOrder::BottomToTop: return "BottomToTop";
        case RowOrder::TopToBottom: return "TopToBottom";
        default: return "BottomToTop";
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

    RowOrder rowOrder = RowOrder::TopToBottom;

    // Ready-made presets
    static TextWriteOptions DefaultPretty();
    static TextWriteOptions DefaultUtility(CommaTabSpace d = CommaTabSpace::Space);

    inline friend std::ostream& operator<<(std::ostream& os, const TextWriteOptions& opt) {
        os << "{axes=" << toString(opt.axes) << ",alphanumeric=" << opt.alphanumeric
            << ",wideNegatives=" << opt.wideNegatives << ",delim=" << toString(opt.delim)
            << ",window=" << opt.window << ",order=" << toString(opt.rowOrder) << "}";
        return os;
    }
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
