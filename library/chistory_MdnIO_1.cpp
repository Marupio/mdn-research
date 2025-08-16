// ================================
// File: library/Mdn2dIO.h
// ================================
#pragma once

#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include "Rect.h"
#include "Mdn2dBase.h"

namespace mdn {

// Text axes rendering style
enum class AxesOutput { None, BoxArt, Simple };

// Options for writing text
struct TextWriteOptions {
    // When axes != None, draw axes crossing at origin (0,0)
    AxesOutput axes = AxesOutput::BoxArt;
    // When true, emit 0-9, a, b, c... for |digit| >= 10; otherwise full integers
    bool alphanumeric = true;
    // When true, pretty mode uses U+2500 "box-art" bar as negative sign in grid body
    bool wideNegatives = true;
    // CSV/TSV/etc for utility mode
    Mdn2dBase::CommaTabSpace delim = Mdn2dBase::CommaTabSpace::Space;
    // Optional clamp window; if invalid -> use bounds() or selection rectangle supplied to writer
    Rect window = Rect::Invalid();
};

// Result of reading text (pretty/utility) back into an MDN
struct TextReadSummary {
    Rect parsedRect = Rect::Invalid(); // bottom-left inclusive to top-right inclusive
    int width = 0;                     // columns
    int height = 0;                    // rows
};

// Binary format header (on-disk)
//  Magic  : 6 bytes  "MDN2D\0"
//  Ver    : uint16   1
//  Base   : int32    base (>=2)
//  Prec   : int32    precision (max span)
//  Sign   : uint8    0=Symmetric, 1=Positive, 2=Negative (matches your enum order)
//  Rect   : 4*int32  x0,y0,x1,y1 (inclusive)
//  H,W    : 2*int32  height (rows), width (cols)
//  Data   : H*W*int8 row-major, y asc (bottom->top), x asc (left->right)

class MDN_API Mdn2dIO {
public:
    // -------- Text I/O --------
    static void saveTextPretty(const Mdn2dBase& src, std::ostream& os,
                               const TextWriteOptions& opt = {});

    static void saveTextUtility(const Mdn2dBase& src, std::ostream& os,
                                const TextWriteOptions& opt = {});

    // Auto-detects pretty/utility by content; clears and writes into dst
    // Returns a summary of parsed dimensions/rect. Throws std::runtime_error on hard parse errors.
    static TextReadSummary loadText(std::istream& is, Mdn2dBase& dst);

    // -------- Binary I/O --------
    static void saveBinary(const Mdn2dBase& src, std::ostream& os);
    static void loadBinary(std::istream& is, Mdn2dBase& dst);

    // -------- Dispatcher --------
    // Sniffs stream; calls loadBinary() if magic matches, else loadText().
    static TextReadSummary load(std::istream& is, Mdn2dBase& dst);
};

// Stream operators (ASCII text). No trailing newline added.
inline std::ostream& operator<<(std::ostream& os, const Mdn2dBase& mdn) {
    TextWriteOptions opt{}; // defaults to pretty BoxArt
    Mdn2dIO::saveTextPretty(mdn, os, opt);
    return os;
}

inline std::istream& operator>>(std::istream& is, Mdn2dBase& mdn) {
    Mdn2dIO::load(is, mdn);
    return is;
}

} // namespace mdn


// ================================
// File: library/Mdn2dIO.cpp
// ================================
#include "Mdn2dIO.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <limits>
#include <sstream>

#include "Coord.h"
#include "Mdn2dConfig.h"
#include "Tools.h" // for digitToAlpha (and maybe constants); we fall back if absent

namespace {

static inline char32_t toLower32(char32_t c) {
    if (c >= U'A' && c <= U'Z') {
        return c - U'A' + U'a';
    }
    return c;
}

static int alphaToDigit(char32_t c) {
    // 0..9 -> 0..9 ; a..z -> 10..35
    if (c >= U'0' && c <= U'9') {
        return int(c - U'0');
    }
    char32_t lc = toLower32(c);
    if (lc >= U'a' && lc <= U'z') {
        return 10 + int(lc - U'a');
    }
    return std::numeric_limits<int>::min(); // sentinel: not a single-digit token
}

static std::string joinDelimited(const std::vector<int>& row, char delim) {
    std::ostringstream oss;
    for (size_t i = 0; i < row.size(); ++i) {
        if (i) {
            oss << delim;
        }
        oss << row[i];
    }
    return oss.str();
}

static inline bool isAxisCharSimple(char32_t c) {
    return c == U'|' || c == U'-' || c == U'+';
}

static inline bool isAxisCharBox(char32_t c) {
    return c == U'│' || c == U'─' || c == U'┼';
}

static inline bool likelyAxisLine(const std::u32string& s) {
    // A line is treated as axis-art if it contains only whitespace and axis glyphs
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
    // naive UTF-8 -> U32 (sufficient for ASCII + box-draw chars used here)
    std::u32string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ) {
        unsigned char c = static_cast<unsigned char>(in[i]);
        if (c < 0x80) {
            out.push_back(c);
            ++i;
        } else if ((c >> 5) == 0x6 && i + 1 < in.size()) {
            char32_t cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(in[i+1]) & 0x3F);
            out.push_back(cp);
            i += 2;
        } else if ((c >> 4) == 0xE && i + 2 < in.size()) {
            char32_t cp = ((c & 0x0F) << 12)
                        | ((static_cast<unsigned char>(in[i+1]) & 0x3F) << 6)
                        |  (static_cast<unsigned char>(in[i+2]) & 0x3F);
            out.push_back(cp);
            i += 3;
        } else if ((c >> 3) == 0x1E && i + 3 < in.size()) {
            char32_t cp = ((c & 0x07) << 18)
                        | ((static_cast<unsigned char>(in[i+1]) & 0x3F) << 12)
                        | ((static_cast<unsigned char>(in[i+2]) & 0x3F) << 6)
                        |  (static_cast<unsigned char>(in[i+3]) & 0x3F);
            out.push_back(cp);
            i += 4;
        } else {
            // skip invalid; keep moving
            ++i;
        }
    }
    return out;
}

static std::string fromU32(const std::u32string& in) {
    // naive U32 -> UTF-8
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

// Tokenize one data line into integers. Supports CSV/TSV/space and pretty (alpha + wide negatives).
static std::vector<int> parseOneRow(const std::string& line) {
    std::u32string u = toU32(line);

    // Quick skip: axis lines
    if (likelyAxisLine(u)) {
        return {};
    }

    // Replace any box-art vertical with space; ASCII '|' with space (split later)
    for (char32_t& c : u) {
        if (c == U'│' || c == U'|') {
            c = U' ';
        }
    }

    std::vector<int> out;
    std::u32string token;
    auto flush = [&]() {
        if (token.empty()) {
            return;
        }
        // Convert token -> int: prefer decimal if multi-char numeric, else alpha-to-digit
        // Normalize any box-art minus to '-'
        for (char32_t& ch : token) {
            if (ch == U'─') {
                ch = U'-';
            }
        }
        // Decide mode
        bool anyAlpha = false;
        bool anyDigit = false;
        for (char32_t ch : token) {
            if ((ch >= U'0' && ch <= U'9') || ch == U'-') {
                anyDigit = true;
            }
            if ((ch >= U'A' && ch <= U'Z') || (ch >= U'a' && ch <= U'z')) {
                anyAlpha = true;
            }
        }
        if (anyAlpha && !anyDigit) {
            // Pure alpha token like "a" or "C" => single cell value
            int v = alphaToDigit(token.front());
            if (v == std::numeric_limits<int>::min()) {
                v = 0;
            }
            out.push_back(v);
        } else {
            // Decimal integer, possibly with leading '-'
            std::string ascii = fromU32(token);
            try {
                int v = std::stoi(ascii);
                out.push_back(v);
            } catch (...) {
                out.push_back(0);
            }
        }
        token.clear();
    };

    // Split by comma, tab, whitespace; keep contiguous digits/letters/negatives together
    for (size_t i = 0; i < u.size(); ++i) {
        char32_t c = u[i];
        bool isSep = (c == U',' || c == U'\t' || c == U' ' || c == U'\r' || c == U'\n');
        if (isSep) {
            flush();
            continue;
        }
        token.push_back(c);
    }
    flush();

    return out;
}

} // anonymous

namespace mdn {

static inline char delimChar(Mdn2dBase::CommaTabSpace d) {
    switch (d) {
        case Mdn2dBase::CommaTabSpace::Comma: return ',';
        case Mdn2dBase::CommaTabSpace::Tab:   return '\t';
        case Mdn2dBase::CommaTabSpace::Space: default: return ' ';
    }
}

void Mdn2dIO::saveTextUtility(const Mdn2dBase& src, std::ostream& os, const TextWriteOptions& opt) {
    Rect b = src.hasBounds() ? src.bounds() : Rect::Invalid();
    Rect w = opt.window.isValid() ? opt.window : b;
    if (!w.isValid()) {
        return; // nothing to write
    }

    const int x0 = w.left();
    const int x1 = w.right();
    const int y0 = w.bottom();
    const int y1 = w.top();

    std::vector<Digit> row;
    row.reserve(size_t(w.width()));

    for (int y = y0; y <= y1; ++y) {
        row.clear();
        bool ok = src.getRowRange(y, x0, x1, row);
        if (!ok) {
            row.assign(size_t(w.width()), Digit(0));
        }
        // convert to integers (Digit is signed small int)
        std::vector<int> ints;
        ints.reserve(row.size());
        for (Digit d : row) {
            ints.push_back(int(d));
        }
        os << joinDelimited(ints, delimChar(opt.delim));
        if (y < y1) {
            os << '\n';
        }
    }
}

void Mdn2dIO::saveTextPretty(const Mdn2dBase& src, std::ostream& os, const TextWriteOptions& opt) {
    Rect b = src.hasBounds() ? src.bounds() : Rect::Invalid();
    Rect w = opt.window.isValid() ? opt.window : b;
    if (!w.isValid()) {
        return;
    }

    const int x0 = w.left();
    const int x1 = w.right();
    const int y0 = w.bottom();
    const int y1 = w.top();

    const char* H = (opt.axes == AxesOutput::BoxArt) ? "\xE2\x94\x80" : "-"; // U+2500
    const char* V = (opt.axes == AxesOutput::BoxArt) ? "\xE2\x94\x82" : "|"; // U+2502
    const char* X = (opt.axes == AxesOutput::BoxArt) ? "\xE2\x94\xBC" : "+"; // U+253C

    std::vector<Digit> row;
    row.reserve(size_t(w.width()));

    for (int y = y0; y <= y1; ++y) {
        row.clear();
        bool ok = src.getRowRange(y, x0, x1, row);
        if (!ok) {
            row.assign(size_t(w.width()), Digit(0));
        }

        // Build one visual row
        std::ostringstream line;
        for (int x = x0; x <= x1; ++x) {
            const Digit d = row[size_t(x - x0)];
            if (opt.alphanumeric && std::abs(int(d)) < 36) {
                // map 0..35 to 0..9,a..z
                int v = std::abs(int(d));
                char ch = (v < 10) ? char('0' + v) : char('a' + (v - 10));
                if (d < 0) {
                    if (opt.wideNegatives) {
                        line << H << ch; // visually aligns with H used for axes
                    } else {
                        line << '-' << ch;
                    }
                } else {
                    line << (v < 10 ? ' ' : ' ') << ch; // keep a single leading space for pos
                }
            } else {
                line << int(d);
            }
            if (x < x1) {
                line << ' ';
            }
        }

        // Inject vertical axis at x==0 if inside window
        if (opt.axes != AxesOutput::None && 0 >= x0 && 0 <= x1) {
            const int col = (0 - x0);
            std::string s = line.str();
            // Find byte index of nth separator; simplistic since we only used ASCII except H
            int spacesSeen = 0;
            size_t insertAt = 0;
            for (size_t i = 0; i < s.size(); ++i) {
                if (s[i] == ' ') {
                    ++spacesSeen;
                }
                if (spacesSeen == col) {
                    insertAt = i;
                    break;
                }
            }
            if (insertAt < s.size()) {
                s.insert(insertAt, V);
            } else {
                s.append(V);
            }
            os << s;
        } else {
            os << line.str();
        }
        os << '\n';
    }

    // Draw horizontal axis (last), if y==0 in window
    if (opt.axes != AxesOutput::None && 0 >= y0 && 0 <= y1) {
        const int widthChars = (w.width() * 2) - 1;
        for (int i = 0; i < widthChars; ++i) {
            os << H;
        }
        // Add cross at x==0 if also within window
        if (0 >= x0 && 0 <= x1) {
            os << X; // simplistic; visual alignment may need tuning later
        }
        // trailing newline intentionally omitted for operator<< etiquette
    }
}

Mdn2dIO::TextReadSummary Mdn2dIO::loadText(std::istream& is, Mdn2dBase& dst) {
    // Read all lines first (keeps implementation simple)
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(is, line)) {
        // Drop BOM if present at very first line
        if (!lines.size() && line.size() >= 3
            && static_cast<unsigned char>(line[0]) == 0xEF
            && static_cast<unsigned char>(line[1]) == 0xBB
            && static_cast<unsigned char>(line[2]) == 0xBF) {
            line.erase(0, 3);
        }
        // Skip obvious metadata footer like "Bounds = [...]"
        if (line.rfind("Bounds =", 0) == 0) {
            continue;
        }
        lines.push_back(line);
    }

    std::vector<std::vector<int>> grid;
    grid.reserve(lines.size());

    for (const std::string& ln : lines) {
        std::vector<int> row = parseOneRow(ln);
        if (!row.empty()) {
            grid.push_back(std::move(row));
        }
    }

    if (grid.empty()) {
        return {};
    }

    // Rect: anchor at bottom-left of pasted grid (0,0) by default
    TextReadSummary out;
    out.height = int(grid.size());
    out.width  = int(grid.front().size());
    out.parsedRect = Rect(0, 0, out.width - 1, out.height - 1);

    // Write into dst starting at (0,0)
    dst.clear();
    for (int r = 0; r < out.height; ++r) {
        std::vector<Digit> row;
        row.reserve(size_t(out.width));
        for (int c = 0; c < out.width; ++c) {
            row.push_back(static_cast<Digit>(grid[size_t(r)][size_t(c)]));
        }
        dst.setRowRange(out.parsedRect.bottom() + r, out.parsedRect.left(), row);
    }

    return out;
}

void Mdn2dIO::saveBinary(const Mdn2dBase& src, std::ostream& os) {
    const char magic[6] = {'M','D','N','2','D','\0'};
    os.write(magic, 6);
    uint16_t ver = 1;
    os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));

    const Mdn2dConfig& cfg = src.getConfig();
    int32_t base = cfg.base();
    int32_t prec = cfg.precision();
    uint8_t sign = static_cast<uint8_t>(cfg.signConvention());

    os.write(reinterpret_cast<const char*>(&base), sizeof(base));
    os.write(reinterpret_cast<const char*>(&prec), sizeof(prec));
    os.write(reinterpret_cast<const char*>(&sign), sizeof(sign));

    Rect b = src.hasBounds() ? src.bounds() : Rect::Invalid();
    int32_t x0 = b.isValid() ? b.left()   : 0;
    int32_t y0 = b.isValid() ? b.bottom() : 0;
    int32_t x1 = b.isValid() ? b.right()  : -1;
    int32_t y1 = b.isValid() ? b.top()    : -1;

    os.write(reinterpret_cast<const char*>(&x0), sizeof(x0));
    os.write(reinterpret_cast<const char*>(&y0), sizeof(y0));
    os.write(reinterpret_cast<const char*>(&x1), sizeof(x1));
    os.write(reinterpret_cast<const char*>(&y1), sizeof(y1));

    int32_t H = b.isValid() ? b.height() : 0;
    int32_t W = b.isValid() ? b.width()  : 0;
    os.write(reinterpret_cast<const char*>(&H), sizeof(H));
    os.write(reinterpret_cast<const char*>(&W), sizeof(W));

    if (!b.isValid()) {
        return; // nothing else to write
    }

    std::vector<Digit> row;
    row.reserve(size_t(W));
    for (int y = y0; y <= y1; ++y) {
        row.clear();
        bool ok = src.getRowRange(y, x0, x1, row);
        if (!ok) {
            row.assign(size_t(W), Digit(0));
        }
        for (Digit d : row) {
            int8_t b8 = static_cast<int8_t>(d);
            os.write(reinterpret_cast<const char*>(&b8), sizeof(b8));
        }
    }
}

void Mdn2dIO::loadBinary(std::istream& is, Mdn2dBase& dst) {
    char magic[6] = {0};
    is.read(magic, 6);
    if (std::memcmp(magic, "MDN2D\0", 6) != 0) {
        throw std::runtime_error("Invalid MDN2D binary magic");
    }
    uint16_t ver = 0;
    is.read(reinterpret_cast<char*>(&ver), sizeof(ver));
    if (ver != 1) {
        throw std::runtime_error("Unsupported MDN2D binary version");
    }

    int32_t base = 0, prec = 0; uint8_t sign = 0;
    is.read(reinterpret_cast<char*>(&base), sizeof(base));
    is.read(reinterpret_cast<char*>(&prec), sizeof(prec));
    is.read(reinterpret_cast<char*>(&sign), sizeof(sign));

    int32_t x0=0,y0=0,x1=-1,y1=-1; int32_t H=0,W=0;
    is.read(reinterpret_cast<char*>(&x0), sizeof(x0));
    is.read(reinterpret_cast<char*>(&y0), sizeof(y0));
    is.read(reinterpret_cast<char*>(&x1), sizeof(x1));
    is.read(reinterpret_cast<char*>(&y1), sizeof(y1));
    is.read(reinterpret_cast<char*>(&H), sizeof(H));
    is.read(reinterpret_cast<char*>(&W), sizeof(W));

    // Configure destination
    Mdn2dConfig cfg = dst.getConfig();
    cfg.setBase(base);
    cfg.setPrecision(prec);
    cfg.setSignConvention(static_cast<SignConvention>(sign));
    dst.setConfig(cfg);

    dst.clear();
    if (H <= 0 || W <= 0) {
        return;
    }

    std::vector<Digit> row;
    row.resize(size_t(W), Digit(0));

    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            int8_t b8 = 0;
            is.read(reinterpret_cast<char*>(&b8), sizeof(b8));
            row[size_t(c)] = static_cast<Digit>(b8);
        }
        dst.setRowRange(y0 + r, x0, row);
    }
}

Mdn2dIO::TextReadSummary Mdn2dIO::load(std::istream& is, Mdn2dBase& dst) {
    // Peek to sniff magic without consuming stream for text path
    char head[6] = {0};
    std::streampos pos = is.tellg();
    is.read(head, 6);
    is.clear();
    is.seekg(pos);
    if (std::memcmp(head, "MDN2D\0", 6) == 0) {
        loadBinary(is, dst);
        // Summary for binary path comes from header we just read again
        // Reread (cheap)
        is.seekg(pos);
        char magic[6]; is.read(magic, 6);
        uint16_t ver; is.read(reinterpret_cast<char*>(&ver), sizeof(ver));
        int32_t base, prec; uint8_t sign; is.read(reinterpret_cast<char*>(&base), sizeof(base));
        is.read(reinterpret_cast<char*>(&prec), sizeof(prec));
        is.read(reinterpret_cast<char*>(&sign), sizeof(sign));
        int32_t x0,y0,x1,y1,H,W; is.read(reinterpret_cast<char*>(&x0), sizeof(x0));
        is.read(reinterpret_cast<char*>(&y0), sizeof(y0));
        is.read(reinterpret_cast<char*>(&x1), sizeof(x1));
        is.read(reinterpret_cast<char*>(&y1), sizeof(y1));
        is.read(reinterpret_cast<char*>(&H), sizeof(H));
        is.read(reinterpret_cast<char*>(&W), sizeof(W));
        TextReadSummary out; out.width = W; out.height = H; out.parsedRect = Rect(x0,y0,x1,y1);
        return out;
    }

    return loadText(is, dst);
}

} // namespace mdn


// ================================
// Patch: library/Mdn2dBase.h (minimal additions)
// ================================
/*
--- a/library/Mdn2dBase.h
+++ b/library/Mdn2dBase.h
@@ class Mdn2dBase {
-    friend class Project;
+    friend class Project;
+    friend class Mdn2dIO; // grant I/O helper access to internals if needed later
@@ public:
+        // --- High-level save/load entry points (ASCII text & binary) ---
+        // These defer to Mdn2dIO. Keeping here for discoverability.
+        void saveText(std::ostream& os) const { Mdn2dIO::saveTextPretty(*this, os, {}); }
+        void saveUtility(std::ostream& os, CommaTabSpace d=CommaTabSpace::Space) const {
+            TextWriteOptions opt; opt.delim = d; Mdn2dIO::saveTextUtility(*this, os, opt);
+        }
+        void saveBinary(std::ostream& os) const { Mdn2dIO::saveBinary(*this, os); }
+        void load(std::istream& is) { Mdn2dIO::load(is, *this); }
+        void loadText(std::istream& is) { Mdn2dIO::loadText(is, *this); }
+        void loadBinary(std::istream& is) { Mdn2dIO::loadBinary(is, *this); }
*/

// ================================
// Notes
// ================================
// • operator<< does not append a trailing newline; callers decide. This is conventional.
// • Text parsing aims to be permissive: it skips axis-only lines (both simple and box-art),
//   normalizes the wide-minus (U+2500) to '-', and supports single‑char alphanumerics.
// • Binary is compact and fast for round‑tripping whole MDNs; TSV remains the best external
//   interchange format (Excel, etc.).
// • All control statements use braces, and multi-line calls are indented by one level per your style.
