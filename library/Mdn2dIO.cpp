#include "Mdn2dIO.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <limits>
#include <sstream>

#include "Coord.h"
#include "Logger.h"
#include "Mdn2dConfig.h"
#include "Tools.h"

namespace {

// Forward declaration so helpers can use toU32
static std::u32string toU32(const std::string& in);

// Use Tools' UTF-8 box-drawing strings; decode once to char32_t codepoints
static inline char32_t boxCpFromUtf8(const std::string& s, char32_t fallback) {
    std::u32string u = toU32(s);
    return u.empty() ? fallback : u[0];
}


static inline char32_t BOX_V() { // │
    static const char32_t cp = boxCpFromUtf8(mdn::Tools::BoxArtStr_v, U'\u2502');
    return cp;
}


static inline char32_t BOX_H() { // ─
    static const char32_t cp = boxCpFromUtf8(mdn::Tools::BoxArtStr_h, U'\u2500');
    return cp;
}


static inline char32_t BOX_X() { // ┼
    static const char32_t cp = boxCpFromUtf8(mdn::Tools::BoxArtStr_x, U'\u253C');
    return cp;
}


static inline char32_t toLower32(char32_t c) {
    if (c >= U'A' && c <= U'Z') {
        return c - U'A' + U'a';
    }
    return c;
}


static int alphaToDigit(char32_t c) {
    if (c >= U'0' && c <= U'9') {
        return int(c - U'0');
    }
    char32_t lc = toLower32(c);
    if (lc >= U'a' && lc <= U'z') {
        return 10 + int(lc - U'a');
    }
    return std::numeric_limits<int>::min();
}


static std::string joinDelimited(const std::vector<int>& row, char delim) {
    std::ostringstream oss;
    if (!row.empty()) {
        oss << row[0];
    }
    for (std::size_t i = 1; i < row.size(); ++i) {
        oss << delim << row[i];
    }
    return oss.str();
}


static inline bool isAxisCharSimple(char32_t c) {
    return c == U'|' || c == U'-' || c == U'+';
}


static inline bool isAxisCharBox(char32_t c) {
    return c == BOX_V() || c == BOX_H() || c == BOX_X();
}


static inline bool likelyAxisLine(const std::u32string& s) {
    for (char32_t c : s) {
        if (c == U' ' || c == U'\t' || c == U'\r') {
            continue;
        }
        if (!isAxisCharSimple(c) && !isAxisCharBox(c)) {
            return false;
        }
    }
    return !s.empty();
}


static std::u32string toU32(const std::string& in) {
    std::u32string out;
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size();) {
        unsigned char c = static_cast<unsigned char>(in[i]);
        if (c < 0x80) {
            out.push_back(c);
            ++i;
        } else if ((c >> 5) == 0x6 && i + 1 < in.size()) {
            char32_t cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(in[i + 1]) & 0x3F);
            out.push_back(cp);
            i += 2;
        } else if ((c >> 4) == 0xE && i + 2 < in.size()) {
            char32_t cp = ((c & 0x0F) << 12)
                | ((static_cast<unsigned char>(in[i + 1]) & 0x3F) << 6)
                | (static_cast<unsigned char>(in[i + 2]) & 0x3F);
            out.push_back(cp);
            i += 3;
        } else if ((c >> 3) == 0x1E && i + 3 < in.size()) {
            char32_t cp = ((c & 0x07) << 18)
                | ((static_cast<unsigned char>(in[i + 1]) & 0x3F) << 12)
                | ((static_cast<unsigned char>(in[i + 2]) & 0x3F) << 6)
                | (static_cast<unsigned char>(in[i + 3]) & 0x3F);
            out.push_back(cp);
            i += 4;
        } else {
            ++i;
        }
    }
    return out;
}


static std::string fromU32(const std::u32string& in) {
    std::string out;
    for (char32_t c : in) {
        if (c < 0x80) {
            out.push_back(static_cast<char>(c));
        } else if (c < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (c >> 6)));
            out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else if (c < 0x10000) {
            out.push_back(static_cast<char>(0xE0 | (c >> 12)));
            out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xF0 | (c >> 18)));
            out.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        }
    }
    return out;
}


static char delimChar(mdn::CommaTabSpace d) {
    switch (d) {
        case mdn::CommaTabSpace::Comma: return ',';
        case mdn::CommaTabSpace::Tab:   return '\t';
        case mdn::CommaTabSpace::Space: default: return ' ';
    }
}

} // anonymous


void mdn::Mdn2dIO::internal_saveTextHeader(
    const Mdn2dBase& src,
    std::ostream& os
) {
    // Output this format for its header, before the data:
    //  Mdn2d{nameOfMdn2d}
    //  Bounds[(x0,y0)->(x1,y1)] (or bounds could also be 'Bounds[Empty]')
    //  Config(b:10, p:16, s:Positive, c:20, f:X)
    os << "Mdn2d{" << src.m_name << "}\n";
    os << "Bounds" << src.locked_bounds() << '\n';
    os << "Config" << src.locked_config() << '\n';
}


bool mdn::Mdn2dIO::internal_parseNameLine(const std::string& line, std::string& outName) {
    // Expected: Mdn2d{MyName}
    if (line.rfind("Mdn2d{", 0) != 0) return false;
    const auto open = line.find('{');
    const auto close = line.rfind('}');
    if (open == std::string::npos || close == std::string::npos || close <= open+1) return false;
    outName = line.substr(open + 1, close - open - 1);
    return true;
}

bool mdn::Mdn2dIO::internal_parseBoundsLine(
    const std::string& line, int& x0, int& y0, int& x1, int& y1, bool& empty
) {
    Log_Debug3_H("");
    // Expected: Bounds[(x0,y0)->(x1,y1)]  OR  Bounds[Empty]
    if (line.rfind("Bounds[", 0) != 0) {
        Log_Debug3_T("");
        return false;
    }
    if (line.find("Empty") != std::string::npos) {
        empty = true;
        Log_Debug3_T("");
        return true;
    }
    empty = false;
    int tx0, ty0, tx1, ty1;
    // super lightweight parse:
    const char* s = line.c_str();
    if (std::sscanf(s, "Bounds[(%d,%d)->(%d,%d)]", &tx0, &ty0, &tx1, &ty1) == 4) {
        x0 = tx0; y0 = ty0; x1 = tx1; y1 = ty1;
        Log_Debug3_T("");
        return true;
    }
    Log_Debug3_T("");
    return false;
}


bool mdn::Mdn2dIO::internal_parseConfigLine(
    const std::string& line, int& base, int& prec, SignConvention& sign
) {
    // Expected: Config(b:10, p:16, s:Positive, c:20, f:X)
    // Only b,p,s are required for IO; others can be ignored or defaulted.
    if (line.rfind("Config(", 0) != 0) return false;
    base = 10;
    prec = 16;
    sign = SignConvention::Invalid;
    // base
    auto bpos = line.find("b:");
    if (bpos != std::string::npos) base = std::atoi(line.c_str() + bpos + 2);
    // precision
    auto ppos = line.find("p:");
    if (ppos != std::string::npos) prec = std::atoi(line.c_str() + ppos + 2);
    // sign
    auto spos = line.find("s:");
    if (spos != std::string::npos) {
        std::string sstr = line.substr(spos, std::string::npos);
        auto scpos = sstr.find(",");
        if (scpos != std::string::npos) {
            std::string scName = sstr.substr(2, scpos-2);
            sign = NameToSignConvention(scName, false);
        }
    }
    Log_Debug3("base=" << base << ",prec=" << prec << ",sign=" << sign);
    return true;
}


std::vector<std::string> mdn::Mdn2dIO::toStringRows(
    const Mdn2dBase& src,
    const TextWriteOptions& opt
) {
    auto lock = src.lockReadOnly();
    return locked_toStringRows(src, opt);
}


std::vector<std::string> mdn::Mdn2dIO::locked_toStringRows(
    const Mdn2dBase& src,
    const TextWriteOptions& opt
) {
    Log_Debug3_H(src.name());
    Rect b = src.locked_hasBounds() ? src.locked_bounds() : Rect::GetInvalid();
    Rect w = opt.window.isValid() ? opt.window : b;
    std::vector<std::string> lines;
    if (!w.isValid()) {
        Log_Debug3_T("Empty window");
        return lines;
    }

    const int x0 = w.left();
    const int x1 = w.right();
    const int y0 = w.bottom();
    const int y1 = w.top();

    const int xCount = w.width();
    const int yCount = w.height();

    Log_Debug3(
        "w=" << w
        << ", x:(" << x0 << "," << x1 << ")=" << xCount
        << ", y::(" << y0 << "," << y1 << ")=" << yCount
    );

    std::string H = "";
    std::string V = "";
    std::string X = "";
    bool hasAxes = false;
    switch (opt.axes) {
        case AxesOutput::None: {
            break;
        }
        case AxesOutput::BoxArt: {
            hasAxes = true;
            H = Tools::BoxArtStr_h;
            V = Tools::BoxArtStr_v;
            X = Tools::BoxArtStr_x;
            break;
        }
        case AxesOutput::Simple: {
            hasAxes = true;
            H = "-";
            V = "|";
            X = "+";
            break;
        }
    }

    std::string delimStr = mdn::toString(opt.delim);

    // Pad with spaces only when delim is Space
    int signAndDigitPadding = 0;
    if (opt.delim == CommaTabSpace::Space) {
        if (!opt.alphanumeric && src.locked_config().base() > 10) {
            // sign and digit may be 3 characters: e.g. -12
            signAndDigitPadding = 3;
        } else {
            // sign and digit will never exceed 2 characters: e.g. -c
            signAndDigitPadding = 2;
        }
    }

    // DigLine - digit line appears before what index in 'row' array below
    int xDigLine = -x0;
    int yDigLine = 0;

    std::string negativeStr = opt.wideNegatives? Tools::BoxArtStr_h : "-";

    std::vector<Digit> row;
    Log_Debug3("Reserving " << xCount);
    row.reserve(static_cast<std::size_t>(xCount));

    for (int y = y0; y <= y1; ++y) {
        // First, if axes are on, are we at the yDigit line?
        if (hasAxes && (y == yDigLine)) {
            std::string hAssemble;
            if (signAndDigitPadding > 0) {
                for (int i = 0; i <= signAndDigitPadding; ++i) {
                    hAssemble += H;
                }
            } else {
                // No character alignment, default to width of 2
                hAssemble = H + H;
            }
            // i indexes along a row i=0 where x=x0, i.e. x = x0+i
            int i;
            std::ostringstream oss;
            for (i = 0; i < xDigLine && i < xCount; ++i) {
                oss << hAssemble;
            }
            if (i == xDigLine) {
                oss << X << H;
                // extra 'H' is to account for adding a delimeter after the vertical digit line
            }
            for (; i < xCount; ++i) {
                oss << hAssemble;
            }
            lines.push_back(oss.str());
        }
        Log_Debug3_H("getRow dispatch(" << y << "," << x0 << "," << x1 << ", row)");
        src.locked_getRow(y, x0, x1, row);
        Log_Debug3_T(
            "row.size()=" << row.size() << ", xCount=" << xCount << ", row.size()==xCount="
                << (row.size() == xCount)
        );
        Assert(row.size() == xCount, "Rows are not the expected size");
        std::ostringstream line;

        // Now indexing by position in the row array
        int i;
        for (i = 0; i < xDigLine && i < xCount; ++i) {
            std::string digStr(
                Tools::digitToAlpha(
                    row[i],
                    opt.alphanumeric,
                    "",
                    negativeStr,
                    signAndDigitPadding
                )
            );
            line << digStr << delimStr;
        }
        if (hasAxes && (i == xDigLine)) {
            line << V << delimStr;
        }
        for (; i < xCount; ++i) {
            std::string digStr(
                Tools::digitToAlpha(
                    row[i],
                    opt.alphanumeric,
                    "",
                    negativeStr,
                    signAndDigitPadding
                )
            );
            line << digStr << delimStr;
        }
        lines.push_back(line.str());
    } // end y loop
    Log_Debug3_T("returning " << lines.size() << " lines of text");
    return lines;
}


std::vector<std::string> mdn::Mdn2dIO::toStringCols(
    const Mdn2dBase& src,
    const TextWriteOptions& opt
) {
    auto lock = src.lockReadOnly();
    return locked_toStringCols(src, opt);
}


std::vector<std::string> mdn::Mdn2dIO::locked_toStringCols(
    const Mdn2dBase& src,
    const TextWriteOptions& opt
) {
    Log_Debug3_H(src.locked_name());
    Rect b = src.locked_hasBounds() ? src.locked_bounds() : Rect::GetInvalid();
    Rect w = opt.window.isValid() ? opt.window : b;
    std::vector<std::string> lines;
    if (!w.isValid()) {
        Log_Debug3_T("Empty window");
        return lines;
    }

    const int x0 = w.left();
    const int x1 = w.right();
    const int y0 = w.bottom();
    const int y1 = w.top();

    const int xCount = w.width();
    const int yCount = w.height();

    Log_Debug3(
        "w=" << w
        << ", x:(" << x0 << "," << x1 << ")=" << xCount
        << ", y::(" << y0 << "," << y1 << ")=" << yCount
    );

    std::string H = "";
    std::string V = "";
    std::string X = "";
    bool hasAxes = false;
    switch (opt.axes) {
        case AxesOutput::None: {
            break;
        }
        case AxesOutput::BoxArt: {
            hasAxes = true;
            H = Tools::BoxArtStr_h;
            V = Tools::BoxArtStr_v;
            X = Tools::BoxArtStr_x;
            break;
        }
        case AxesOutput::Simple: {
            hasAxes = true;
            H = "-";
            V = "|";
            X = "+";
            break;
        }
    }

    std::string delimStr = mdn::toString(opt.delim);

    // Pad with spaces only when delim is Space
    int signAndDigitPadding = 0;
    if (opt.delim == CommaTabSpace::Space) {
        if (!opt.alphanumeric && src.locked_config().base() > 10) {
            // sign and digit may be 3 characters: e.g. -12
            signAndDigitPadding = 3;
        } else {
            // sign and digit will never exceed 2 characters: e.g. -c
            signAndDigitPadding = 2;
        }
    }

    // DigLine - digit line appears before what index in 'col' array below
    int yDigLine = -y0;
    int xDigLine = 0;

    std::string negativeStr = opt.wideNegatives? Tools::BoxArtStr_h : "-";

    std::vector<Digit> col;
    col.reserve(static_cast<std::size_t>(yCount));

    for (int x = x0; x <= x1; ++x) {
        // First, if axes are on, are we at the xDigit line?
        if (hasAxes && (x == xDigLine)) {
            std::string hAssemble;
            if (signAndDigitPadding > 0) {
                for (int i = 0; i <= signAndDigitPadding; ++i) {
                    hAssemble += H;
                }
            } else {
                // No character alignment, default to width of 2
                hAssemble = H + H;
            }
            // i indexes along a col i=0 where y=y0, i.e. y = y0+i
            int i;
            std::ostringstream oss;
            for (i = 0; i < yDigLine && i < yCount; ++i) {
                oss << hAssemble;
            }
            if (i == yDigLine) {
                oss << X << H;
                // extra 'H' is to account for adding a delimeter after the vertical digit line
            }
            for (; i < yCount; ++i) {
                oss << hAssemble;
            }
            lines.push_back(oss.str());
        }
        Log_Debug3_H("getCol dispatch(" << x << "," << x0 << "," << x1 << ", col)");
        src.locked_getCol(x, y0, y1, col);
        Log_Debug3_T(
            "col.size()=" << col.size() << ", yCount=" << yCount << ", col.size()==yCount="
                << (col.size() == yCount)
        );
        Assert(col.size() == yCount, "Cols are not the expected size");
        std::ostringstream line;

        // Now indexing by position in the col array
        int i;
        for (i = 0; i < yDigLine && i < yCount; ++i) {
            std::string digStr(
                Tools::digitToAlpha(
                    col[i],
                    opt.alphanumeric,
                    "",
                    negativeStr,
                    signAndDigitPadding
                )
            );
            line << digStr << delimStr;
        }
        if (hasAxes && (i == yDigLine)) {
            line << V << delimStr;
        }
        for (; i < yCount; ++i) {
            std::string digStr(
                Tools::digitToAlpha(
                    col[i],
                    opt.alphanumeric,
                    "",
                    negativeStr,
                    signAndDigitPadding
                )
            );
            line << digStr << delimStr;
        }
        lines.push_back(line.str());
    } // end x loop
    Log_Debug3_T("returning " << lines.size() << " lines of text");
    return lines;
}


void mdn::Mdn2dIO::saveTextPretty(
    const Mdn2dBase& src,
    std::ostream& os,
    const TextWriteOptions& opt
) {
    auto lock = src.lockReadOnly();
    return locked_saveTextPretty(src, os, opt);
}


void mdn::Mdn2dIO::locked_saveTextPretty(
    const Mdn2dBase& src,
    std::ostream& os,
    const TextWriteOptions& opt
) {
    Log_Debug3_H("");
    // First output header (name and bounds)
    internal_saveTextHeader(src, os);
    // Now output the data
    std::vector<std::string> lines = locked_toStringRows(src, opt);
    for (std::size_t i = 0; i < lines.size(); ++i) {
        os << lines[i];
        if (i + 1 < lines.size()) {
            os << '\n';
        }
    }
    Log_Debug3_T("");
}


void mdn::Mdn2dIO::saveTextUtility(
    const Mdn2dBase& src,
    std::ostream& os,
    const TextWriteOptions& opt
) {
    auto lock = src.lockReadOnly();
    return locked_saveTextUtility(src, os, opt);
}


void mdn::Mdn2dIO::locked_saveTextUtility(
    const Mdn2dBase& src,
    std::ostream& os,
    const TextWriteOptions& opt
) {
    Log_Debug3_H("");
    // First output header (name and bounds)
    internal_saveTextHeader(src, os);
    // Now output the data
    std::vector<std::string> lines = locked_toStringRows(src, opt);
    for (std::size_t i = 0; i < lines.size(); ++i) {
        os << lines[i];
        if (i + 1 < lines.size()) {
            os << '\n';
        }
    }
    Log_Debug3_T("");
}


mdn::TextReadSummary mdn::Mdn2dIO::loadText(
    std::istream& is,
    Mdn2dBase& dst
) {
    auto lock = dst.lockWriteable();
    Log_Debug3("");
    return locked_loadText(is, dst);
}


mdn::TextReadSummary mdn::Mdn2dIO::locked_loadText(
    std::istream& is,
    Mdn2dBase& dst
) {
    Log_Debug3_H("");
    Log_Info("locked_loadText");
    std::string nameLine, boundsLine, configLine;

    // Handle optional BOM on the very first getline
    if (!std::getline(is, nameLine)) {
        Log_Debug3_T("empty stream");
        return {};
    }
    if (
        nameLine.size() >= 3 &&
        static_cast<unsigned char>(nameLine[0]) == 0xEF &&
        static_cast<unsigned char>(nameLine[1]) == 0xBB &&
        static_cast<unsigned char>(nameLine[2]) == 0xBF
    ) {
        nameLine.erase(0, 3);
    }

    if (!std::getline(is, boundsLine)) {
        Log_Debug3_T("Stream ended early");
        return {};
    }
    if (!std::getline(is, configLine)) {
        Log_Debug3_T("Stream ended early");
        return {};
    }

    std::string mdnName;
    if (!internal_parseNameLine(nameLine, mdnName)) {
        Log_Debug3_T("Stream ended early");
        return {};
    }

    int x0=0,y0=0,x1=-1,y1=-1; bool empty=false;
    if (!internal_parseBoundsLine(boundsLine, x0, y0, x1, y1, empty)) {
        Log_Debug3_T("Stream ended early");
        return {};
    }

    int base=10;
    int prec=16;
    SignConvention sign=SignConvention::Invalid;
    if (!internal_parseConfigLine(configLine, base, prec, sign)) {
        Log_Debug3_T("Stream ended early");
        return {};
    }

    // Apply name & config
    dst.m_name = mdnName; // allowed because we're in locked_*; matches how you write it out. :contentReference[oaicite:10]{index=10}
    const Mdn2dConfig& dstCfg = dst.locked_config();
    Mdn2dConfig cfg(
        base, prec, static_cast<SignConvention>(sign),
        dstCfg.maxCarryoverIters(),
        dstCfg.fraxis()
    );
    dst.locked_setConfig(cfg);
    // Clear and set bounds anchor
    dst.locked_clear();
    mdn::Rect writeRect = empty ? mdn::Rect::GetInvalid() : mdn::Rect(x0, y0, x1, y1, true);

    // Now read the remainder of the stream as data lines (your existing logic below)
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(is, line)) {
        lines.push_back(line);
    }

    std::vector<std::vector<int>> grid;
    grid.reserve(lines.size());

    for (const std::string& ln : lines) {
        std::u32string u = toU32(ln);
        if (likelyAxisLine(u)) {
            continue;
        }
        for (char32_t& c : u) {
            if (c == BOX_V() || c == U'|') {
                c = U' ';
            }
            if (c == BOX_H()) {
                c = U'-';
            }
        }
        std::string ascii = fromU32(u);

        std::istringstream iss(ascii);
        std::string cell;
        std::vector<int> row;
        while (std::getline(iss, cell, ' ')) {
            if (cell.empty()) {
                continue;
            }
            bool hasAlpha = false;
            for (char ch : cell) {
                if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')) {
                    hasAlpha = true;
                    break;
                }
            }
            if (hasAlpha && cell.size() == 1) {
                int v = alphaToDigit(static_cast<char32_t>(cell[0]));
                row.push_back(v == std::numeric_limits<int>::min() ? 0 : v);
            } else {
                try {
                    int v = std::stoi(cell);
                    row.push_back(v);
                } catch (...) {
                    row.push_back(0);
                }
            }
        }
        if (!row.empty()) {
            grid.push_back(std::move(row));
        }
    }

    TextReadSummary out;
    if (grid.empty()) {
        Log_Debug3_T("result = " << out);
        return out;
    }

    const int H = static_cast<int>(grid.size());
    const int W = static_cast<int>(grid.front().size());

    // Destination anchor (bottom-left) from header, or (0,0) if Empty/invalid
    const int ax = writeRect.isValid() ? writeRect.left()   : 0;
    const int ay = writeRect.isValid() ? writeRect.bottom() : 0;

    // Clear and write rows, anchored at (ax, ay)
    dst.locked_clear();

    std::vector<Digit> row;
    row.resize(static_cast<std::size_t>(W));

    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            row[static_cast<std::size_t>(c)] =
                static_cast<Digit>(grid[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)]);
        }
        dst.locked_setRow(ay + r, ax, row);
        // dst.locked_setRow(ay + (H - 1 - r), ax, row);
        Log_Info("row index is " << (ay+r) << ", and other one would be " << ay + (H - 1 - r));
    }

    // Report what we parsed/wrote
    out.width  = W;
    out.height = H;
    out.parsedRect = Rect(ax, ay, ax + W - 1, ay + H - 1);

    Log_Debug3_T("result = " << out);
    return out;
}


void mdn::Mdn2dIO::saveBinary(
    const Mdn2dBase& src,
    std::ostream& os
) {
    Log_Debug3_H("");
    const char magic[6] = {'M','D','N','2','D','\0'};
    os.write(magic, 6);

    // version
    uint16_t ver = 1;
    os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));

    const std::string nameUtf8 = src.m_name;
    uint32_t nameLen = static_cast<uint32_t>(nameUtf8.size());
    os.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
    if (nameLen) {
        os.write(nameUtf8.data(), nameLen);
    }

    // existing: write config
    int32_t base      = std::clamp(src.locked_config().base(), 2, 32);
    int32_t precision = src.locked_config().precision();
    uint8_t sign      = static_cast<uint8_t>(src.locked_config().signConvention());
    os.write(reinterpret_cast<const char*>(&base),      sizeof(base));
    os.write(reinterpret_cast<const char*>(&precision), sizeof(precision));
    os.write(reinterpret_cast<const char*>(&sign),      sizeof(sign));

    Rect b = src.hasBounds() ? src.bounds() : Rect::GetInvalid();

    std::int32_t x0 = b.isValid() ? b.left() : 0;
    std::int32_t y0 = b.isValid() ? b.bottom() : 0;
    std::int32_t x1 = b.isValid() ? b.right() : -1;
    std::int32_t y1 = b.isValid() ? b.top() : -1;

    os.write(reinterpret_cast<const char*>(&x0), sizeof(x0));
    os.write(reinterpret_cast<const char*>(&y0), sizeof(y0));
    os.write(reinterpret_cast<const char*>(&x1), sizeof(x1));
    os.write(reinterpret_cast<const char*>(&y1), sizeof(y1));

    std::int32_t H = b.isValid() ? b.height() : 0;
    std::int32_t W = b.isValid() ? b.width() : 0;

    os.write(reinterpret_cast<const char*>(&H), sizeof(H));
    os.write(reinterpret_cast<const char*>(&W), sizeof(W));

    if (!b.isValid()) {
        Log_Debug3_T("");
        return;
    }

    std::vector<Digit> row;
    row.reserve(static_cast<std::size_t>(W));

    for (int y = y0; y <= y1; ++y) {
        row.clear();
        src.getRow(y, x0, x1, row);
        for (Digit d : row) {
            std::int8_t b8 = static_cast<std::int8_t>(d);
            os.write(reinterpret_cast<const char*>(&b8), sizeof(b8));
        }
    }
    Log_Debug3_T("");
}


void mdn::Mdn2dIO::loadBinary(
    std::istream& is,
    Mdn2dBase& dst
) {
    auto lock = dst.lockWriteable();
    return locked_loadBinary(is, dst);
}


void mdn::Mdn2dIO::locked_loadBinary(
    std::istream& is,
    Mdn2dBase& dst
) {
    Log_Debug3_H("");
    char magic[6] = {0};
    is.read(magic, 6);
    if (std::memcmp(magic, "MDN2D\0", 6) != 0) {
        std::runtime_error err("Invalid MDN2D binary magic");
        Log_Error(err.what());
        throw err;
    }

    std::uint16_t ver = 0;
    is.read(reinterpret_cast<char*>(&ver), sizeof(ver));
    if (ver != 1) {
        std::runtime_error err("Unsupported MDN2D binary version");
        Log_Error(err.what());
        throw err;
    }

    std::uint32_t nameLen = 0;
    is.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));

    std::string nameUtf8;
    nameUtf8.resize(nameLen);
    if (nameLen) {
        is.read(nameUtf8.data(), nameLen);
    }

    dst.m_name = nameUtf8;

    std::int32_t base32 = 0;
    std::int32_t prec32 = 0;
    std::uint8_t sign8 = 0;

    is.read(reinterpret_cast<char*>(&base32), sizeof(base32));
    is.read(reinterpret_cast<char*>(&prec32), sizeof(prec32));
    is.read(reinterpret_cast<char*>(&sign8), sizeof(sign8));

    if (base32 < 2 || base32 > 32) {
        std::runtime_error err("Unsupported base in MDN2D binary (must be 2..32)");
        Log_Error(err.what());
        throw err;
    }

    std::int32_t x0 = 0;
    std::int32_t y0 = 0;
    std::int32_t x1 = -1;
    std::int32_t y1 = -1;
    std::int32_t H = 0;
    std::int32_t W = 0;

    is.read(reinterpret_cast<char*>(&x0), sizeof(x0));
    is.read(reinterpret_cast<char*>(&y0), sizeof(y0));
    is.read(reinterpret_cast<char*>(&x1), sizeof(x1));
    is.read(reinterpret_cast<char*>(&y1), sizeof(y1));
    is.read(reinterpret_cast<char*>(&H), sizeof(H));
    is.read(reinterpret_cast<char*>(&W), sizeof(W));

    Mdn2dConfig dstCfg = dst.locked_config();
    Mdn2dConfig cfg = Mdn2dConfig(
        static_cast<int>(base32),
        static_cast<int>(prec32),
        static_cast<SignConvention>(sign8),
        dstCfg.maxCarryoverIters(),
        dstCfg.fraxis()
    );

    dst.locked_setConfig(cfg);

    if (H <= 0 || W <= 0) {
        Log_Debug3_T("");
        return;
    }

    std::vector<Digit> row;
    row.resize(static_cast<std::size_t>(W), Digit(0));

    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            std::int8_t b8 = 0;
            is.read(reinterpret_cast<char*>(&b8), sizeof(b8));
            row[static_cast<std::size_t>(c)] = static_cast<Digit>(b8);
        }
        dst.locked_setRow(y0 + r, x0, row);
    }
    Log_Debug3_T("");
}


mdn::TextReadSummary mdn::Mdn2dIO::load(
    std::istream& is,
    Mdn2dBase& dst
) {
    auto lock = dst.lockWriteable();
    return locked_load(is, dst);
}


mdn::TextReadSummary mdn::Mdn2dIO::locked_load(
    std::istream& is,
    Mdn2dBase& dst
) {
    Log_Debug3_H("");

    // Remember where parsing should begin
    const std::streampos pos = is.tellg();

    // --- Sniff: try to read the 6-byte magic ---
    char head[6] = {0};
    is.read(head, 6);
    const std::streamsize n = is.gcount();

    // Always restore state & position before deciding what to do next
    is.clear();                 // clear eof/fail so seekg succeeds
    is.seekg(pos);

    const bool looksBinary = (n == 6) && (std::memcmp(head, "MDN2D\0", 6) == 0);

    if (looksBinary) {
        // --- Non-destructive peek of the binary header to build the summary ---
        // If anything below fails, we’ll fall back to text load.
        TextReadSummary out{};
        bool headerOk = true;

        // Re-seek to start of payload for a careful peek
        is.clear();
        is.seekg(pos);

        // Helper to safely read POD and record failure without throwing here
        auto safeRead = [&](void* dstBuf, std::size_t nbytes) {
            is.read(reinterpret_cast<char*>(dstBuf), static_cast<std::streamsize>(nbytes));
            if (!is) { headerOk = false; is.clear(); }
        };

        // 1) Magic (6) — already checked, but advance cursor
        char magic[6] = {0};
        safeRead(magic, sizeof(magic));
        if (!(headerOk && std::memcmp(magic, "MDN2D\0", 6) == 0)) headerOk = false;

        // 2) Version (uint16)
        std::uint16_t ver = 0;
        if (headerOk) safeRead(&ver, sizeof(ver));
        // Don’t throw here; let the real binary loader validate/throw.
        // If unexpected, we’ll just fall back to text.
        if (!(headerOk && ver == 1)) headerOk = false;

        // 3) Name length + name bytes
        std::uint32_t nameLen = 0;
        if (headerOk) safeRead(&nameLen, sizeof(nameLen));
        if (headerOk && nameLen) {
            // Skip over the name bytes
            is.seekg(static_cast<std::streamoff>(nameLen), std::ios::cur);
            if (!is) { headerOk = false; is.clear(); }
        }

        // 4) Config triplet: base (int32), precision (int32), sign (uint8)
        std::int32_t base32 = 0, prec32 = 0;
        std::uint8_t sign8 = 0;
        if (headerOk) safeRead(&base32, sizeof(base32));
        if (headerOk) safeRead(&prec32, sizeof(prec32));
        if (headerOk) safeRead(&sign8,  sizeof(sign8));

        // 5) Rect + size: x0,y0,x1,y1,H,W (all int32)
        std::int32_t x0 = 0, y0 = 0, x1 = -1, y1 = -1, H = 0, W = 0;
        if (headerOk) safeRead(&x0, sizeof(x0));
        if (headerOk) safeRead(&y0, sizeof(y0));
        if (headerOk) safeRead(&x1, sizeof(x1));
        if (headerOk) safeRead(&y1, sizeof(y1));
        if (headerOk) safeRead(&H,  sizeof(H));
        if (headerOk) safeRead(&W,  sizeof(W));

        // Regardless of peek success, restore position before real parse
        is.clear();
        is.seekg(pos);

        if (headerOk) {
            // Do the *real* binary load (consumes stream)
            locked_loadBinary(is, dst);

            // Build an honest summary from the header we peeked
            out.width      = static_cast<int>(W);
            out.height     = static_cast<int>(H);
            out.parsedRect = Rect(x0, y0, x1, y1);
            Log_Debug3_T("result = " << out);
            return out;
        }

        // If header peek failed, fall back to text loader
        Log_Warn("Binary header peek failed, falling back to text parser.");
    }

    // Text path (or binary sniff failed)
    auto result = locked_loadText(is, dst);
    Log_Debug3_T("result = " << result);
    return result;
}
