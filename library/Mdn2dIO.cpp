#include "Mdn2dIO.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <limits>
#include <sstream>

#include "Coord.h"
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


std::vector<std::string> mdn::Mdn2dIO::toStringRows(
    const Mdn2dBase& src,
    const TextWriteOptions& opt
) {
    Rect b = src.hasBounds() ? src.bounds() : Rect::GetInvalid();
    Rect w = opt.window.isValid() ? opt.window : b;
    std::vector<std::string> lines;
    if (!w.isValid()) {
        return lines;
    }

    const int x0 = w.left();
    const int x1 = w.right();
    const int y0 = w.bottom();
    const int y1 = w.top();

    const std::string& H = (opt.axes == AxesOutput::BoxArt) ? Tools::BoxArtStr_h : std::string("-");
    const std::string& V = (opt.axes == AxesOutput::BoxArt) ? Tools::BoxArtStr_v : std::string("|");
    const std::string& X = (opt.axes == AxesOutput::BoxArt) ? Tools::BoxArtStr_x : std::string("+");

    std::vector<Digit> row;
    row.reserve(static_cast<std::size_t>(w.width()));

    for (int y = y0; y <= y1; ++y) {
        row.clear();
        src.getRow(y, x0, x1, row);

        std::ostringstream line;
        for (int x = x0; x <= x1; ++x) {
            const Digit d = row[static_cast<std::size_t>(x - x0)];
            if (opt.alphanumeric && std::abs(static_cast<int>(d)) < 36) {
                int v = std::abs(static_cast<int>(d));
                char ch = (v < 10) ? static_cast<char>('0' + v) : static_cast<char>('a' + (v - 10));
                if (d < 0) {
                    if (opt.wideNegatives) {
                        line << H << ch;
                    } else {
                        line << '-' << ch;
                    }
                } else {
                    line << ch;
                }
            } else {
                line << static_cast<int>(d);
            }
            if (x < x1) {
                line << (opt.delim == CommaTabSpace::Comma ? ','
                         : opt.delim == CommaTabSpace::Tab   ? '\t'
                         : ' ');
            }
        }

        if (opt.axes != AxesOutput::None && 0 >= x0 && 0 <= x1) {
            const int col = (0 - x0);
            std::string s = line.str();
            int seps = 0;
            std::size_t insertAt = s.size();
            for (std::size_t i = 0; i < s.size(); ++i) {
                if (s[i] == ',' || s[i] == '\t' || s[i] == ' ') {
                    ++seps;
                }
                if (seps == col) {
                    insertAt = i;
                    break;
                }
            }
            s.insert(insertAt, V);
            lines.push_back(std::move(s));
        } else {
            lines.push_back(line.str());
        }
    }

    if (opt.axes != AxesOutput::None && 0 >= y0 && 0 <= y1) {
        const int widthChars = (w.width() * 2) - 1;
        std::string axis;
        for (int i = 0; i < widthChars; ++i) {
            axis += H;
        }
        if (0 >= x0 && 0 <= x1) {
            axis += X;
        }
        lines.push_back(std::move(axis));
    }

    return lines;
}

std::vector<std::string> mdn::Mdn2dIO::toStringCols(
    const Mdn2dBase& src,
    const TextWriteOptions& opt
) {
    Rect b = src.hasBounds() ? src.bounds() : Rect::GetInvalid();
    Rect w = opt.window.isValid() ? opt.window : b;
    std::vector<std::string> cols;
    if (!w.isValid()) {
        return cols;
    }

    const int x0 = w.left();
    const int x1 = w.right();
    const int y0 = w.bottom();
    const int y1 = w.top();

    // Build a buffer of rows once, then emit columns
    std::vector<std::vector<Digit>> rowsBuf;
    rowsBuf.reserve(static_cast<std::size_t>(w.height()));

    std::vector<Digit> row;
    row.reserve(static_cast<std::size_t>(w.width()));

    for (int y = y0; y <= y1; ++y) {
        row.clear();
        src.getRow(y, x0, x1, row);
        rowsBuf.push_back(row);
    }

    for (int x = x0; x <= x1; ++x) {
        std::vector<int> col;
        col.reserve(static_cast<std::size_t>(w.height()));
        for (int y = y0; y <= y1; ++y) {
            Digit d = rowsBuf[static_cast<std::size_t>(y - y0)][static_cast<std::size_t>(x - x0)];
            if (opt.alphanumeric && std::abs(static_cast<int>(d)) < 36) {
                int v = std::abs(static_cast<int>(d));
                char ch = (v < 10) ? static_cast<char>('0' + v) : static_cast<char>('a' + (v - 10));
                // represent alphas numerically for columns to keep delimited shape
                int signedVal = (d < 0) ? -v : v;
                col.push_back(signedVal);
            } else {
                col.push_back(static_cast<int>(d));
            }
        }
        cols.push_back(joinDelimited(col, delimChar(opt.delim)));
    }

    return cols;
}

void mdn::Mdn2dIO::saveTextUtility(
    const Mdn2dBase& src,
    std::ostream& os,
    const TextWriteOptions& opt
) {
    std::vector<std::string> lines = toStringRows(src, opt);
    for (std::size_t i = 0; i < lines.size(); ++i) {
        os << lines[i];
        if (i + 1 < lines.size()) {
            os << '\n';
        }
    }
}

void mdn::Mdn2dIO::saveTextPretty(
    const Mdn2dBase& src,
    std::ostream& os,
    const TextWriteOptions& opt
) {
    std::vector<std::string> lines = toStringRows(src, opt);
    for (std::size_t i = 0; i < lines.size(); ++i) {
        os << lines[i];
        if (i + 1 < lines.size()) {
            os << '\n';
        }
    }
}

mdn::TextReadSummary mdn::Mdn2dIO::loadText(
    std::istream& is,
    Mdn2dBase& dst
) {
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(is, line)) {
        if (!lines.size() && line.size() >= 3
            && static_cast<unsigned char>(line[0]) == 0xEF
            && static_cast<unsigned char>(line[1]) == 0xBB
            && static_cast<unsigned char>(line[2]) == 0xBF) {
            line.erase(0, 3);
        }
        if (line.rfind("Bounds =", 0) == 0) {
            continue;
        }
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
        return out;
    }

    out.height = static_cast<int>(grid.size());
    out.width = static_cast<int>(grid.front().size());
    out.parsedRect = Rect(0, 0, out.width - 1, out.height - 1);

    dst.clear();
    for (int r = 0; r < out.height; ++r) {
        std::vector<Digit> row;
        row.reserve(static_cast<std::size_t>(out.width));
        for (int c = 0; c < out.width; ++c) {
            row.push_back(static_cast<Digit>(grid[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)]));
        }
        dst.setRow(out.parsedRect.bottom() + r, out.parsedRect.left(), row);
    }

    return out;
}

void mdn::Mdn2dIO::saveBinary(
    const Mdn2dBase& src,
    std::ostream& os
) {
    const char magic[6] = {'M','D','N','2','D','\0'};
    os.write(magic, 6);

    std::uint16_t ver = 1;
    os.write(reinterpret_cast<const char*>(&ver), sizeof(ver));

    const Mdn2dConfig& cfg = src.getConfig();

    int base = cfg.base();
    if (base < 2) { base = 2; }
    if (base > 32) { base = 32; }

    std::int32_t base32 = static_cast<std::int32_t>(base);
    std::int32_t prec32 = static_cast<std::int32_t>(cfg.precision());
    std::uint8_t sign8 = static_cast<std::uint8_t>(cfg.signConvention());

    os.write(reinterpret_cast<const char*>(&base32), sizeof(base32));
    os.write(reinterpret_cast<const char*>(&prec32), sizeof(prec32));
    os.write(reinterpret_cast<const char*>(&sign8), sizeof(sign8));

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
}

void mdn::Mdn2dIO::loadBinary(
    std::istream& is,
    Mdn2dBase& dst
) {
    char magic[6] = {0};
    is.read(magic, 6);
    if (std::memcmp(magic, "MDN2D\0", 6) != 0) {
        throw std::runtime_error("Invalid MDN2D binary magic");
    }

    std::uint16_t ver = 0;
    is.read(reinterpret_cast<char*>(&ver), sizeof(ver));
    if (ver != 1) {
        throw std::runtime_error("Unsupported MDN2D binary version");
    }

    std::int32_t base32 = 0;
    std::int32_t prec32 = 0;
    std::uint8_t sign8 = 0;

    is.read(reinterpret_cast<char*>(&base32), sizeof(base32));
    is.read(reinterpret_cast<char*>(&prec32), sizeof(prec32));
    is.read(reinterpret_cast<char*>(&sign8), sizeof(sign8));

    if (base32 < 2 || base32 > 32) {
        throw std::runtime_error("Unsupported base in MDN2D binary (must be 2..32)");
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

    Mdn2dConfig dstCfg = dst.getConfig();
    Mdn2dConfig cfg = Mdn2dConfig(
        static_cast<int>(base32),
        static_cast<int>(prec32),
        static_cast<SignConvention>(sign8),
        dstCfg.maxCarryoverIters(),
        dstCfg.defaultFraxis()
    );

    dst.setConfig(cfg);

    if (H <= 0 || W <= 0) {
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
        dst.setRow(y0 + r, x0, row);
    }
}


mdn::TextReadSummary mdn::Mdn2dIO::load(
    std::istream& is,
    Mdn2dBase& dst
) {
    std::streampos pos = is.tellg();
    char head[6] = {0};
    is.read(head, 6);
    is.clear();
    is.seekg(pos);

    if (std::memcmp(head, "MDN2D\0", 6) == 0) {
        loadBinary(is, dst);

        is.seekg(pos);
        char magic[6];
        is.read(magic, 6);

        std::uint16_t ver = 0;
        is.read(reinterpret_cast<char*>(&ver), sizeof(ver));

        std::int32_t base32 = 0;
        std::int32_t prec32 = 0;
        std::uint8_t sign8 = 0;
        is.read(reinterpret_cast<char*>(&base32), sizeof(base32));
        is.read(reinterpret_cast<char*>(&prec32), sizeof(prec32));
        is.read(reinterpret_cast<char*>(&sign8), sizeof(sign8));

        std::int32_t x0, y0, x1, y1, H, W;
        is.read(reinterpret_cast<char*>(&x0), sizeof(x0));
        is.read(reinterpret_cast<char*>(&y0), sizeof(y0));
        is.read(reinterpret_cast<char*>(&x1), sizeof(x1));
        is.read(reinterpret_cast<char*>(&y1), sizeof(y1));
        is.read(reinterpret_cast<char*>(&H), sizeof(H));
        is.read(reinterpret_cast<char*>(&W), sizeof(W));

        TextReadSummary out;
        out.width = static_cast<int>(W);
        out.height = static_cast<int>(H);
        out.parsedRect = Rect(x0, y0, x1, y1);
        return out;
    }

    return loadText(is, dst);
}
