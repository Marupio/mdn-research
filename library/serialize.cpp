// =============================================================
// library/serialize.cpp — v0 implementation (no Qt)
// =============================================================
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>

#include "serialize.h"

namespace mdn::serialize {

static constexpr const char* kTextMagic = "MDN2D v1"; // ASCII header
static constexpr std::uint32_t kBinMagic = 0x424E444Du; // 'M''D''N''B' little-endian
static constexpr std::uint16_t kBinVersion = 1;

// --------- small helpers -------------------------------------------------

std::string rowToTsv(const std::vector<Digit>& row)
{
    std::ostringstream oss;
    for (std::size_t i = 0; i < row.size(); ++i) {
        if (i > 0) {
            oss << '\t';
        }
        oss << static_cast<int>(row[i]);
    }
    return oss.str();
}

static bool parseTsvLine(const std::string& line, std::vector<Digit>& out)
{
    out.clear();
    std::istringstream iss(line);
    std::string cell;
    while (std::getline(iss, cell, '\t')) {
        try {
            int v = std::stoi(cell);
            if (v < -128) { v = -128; }
            if (v >  127) { v =  127; }
            out.push_back(static_cast<Digit>(v));
        } catch (...) {
            return false;
        }
    }
    return !out.empty();
}

static bool readLine(std::istream& is, std::string& line)
{
    line.clear();
    return static_cast<bool>(std::getline(is, line));
}

// --------- text save/load -----------------------------------------------

bool saveText(std::ostream& os, const Mdn2d& mdn)
{
    if (!mdn.hasBounds()) {
        // Empty: write an empty 0x0 grid at origin
        os << kTextMagic << "\n";
        os << "bounds 0 0 -1 -1\n"; // convention: invalid rect ⇒ no rows
        return static_cast<bool>(os);
    }

    const Rect b = mdn.bounds();
    const int x0 = b.left();
    const int x1 = b.right();
    const int y0 = b.bottom();
    const int y1 = b.top();

    os << kTextMagic << "\n";
    os << "bounds " << x0 << ' ' << y0 << ' ' << x1 << ' ' << y1 << "\n";

    std::vector<Digit> row;
    row.reserve(b.width());

    for (int y = y0; y <= y1; ++y) {
        const bool ok = mdn.getRow(y, x0, x1, row);
        if (!ok) {
            // Fill zeros to maintain rectangularity
            row.assign(static_cast<std::size_t>(b.width()), Digit(0));
        }
        os << rowToTsv(row) << '\n';
    }

    return static_cast<bool>(os);
}

bool loadText(std::istream& is, Mdn2d& mdn)
{
    std::string line;

    if (!readLine(is, line)) {
        return false;
    }
    if (line != kTextMagic) {
        return false;
    }

    if (!readLine(is, line)) {
        return false;
    }

    int x0 = 0, y0 = 0, x1 = -1, y1 = -1;
    {
        std::istringstream hdr(line);
        std::string kw;
        hdr >> kw >> x0 >> y0 >> x1 >> y1;
        if (kw != "bounds") {
            return false;
        }
    }

    mdn.clear();

    if (x1 < x0 || y1 < y0) {
        // Empty grid
        return true;
    }

    const int W = (x1 - x0 + 1);
    const int H = (y1 - y0 + 1);

    std::vector<Digit> row;
    row.reserve(static_cast<std::size_t>(W));

    for (int r = 0; r < H; ++r) {
        if (!readLine(is, line)) {
            return false;
        }
        if (!parseTsvLine(line, row)) {
            return false;
        }
        if (static_cast<int>(row.size()) != W) {
            return false;
        }
        mdn.setRowRange(y0 + r, x0, row);
    }

    return true;
}

std::ostream& operator<<(std::ostream& os, const Mdn2d& mdn)
{
    saveText(os, mdn);
    return os;
}

std::istream& operator>>(std::istream& is, Mdn2d& mdn)
{
    loadText(is, mdn);
    return is;
}

std::ostream& operator<<(std::ostream& os, const Coord& c)
{
    os << '(' << c.x << ',' << c.y << ')';
    return os;
}

std::ostream& operator<<(std::ostream& os, const Rect& r)
{
    os << "Rect[" << r.left() << ',' << r.bottom()
       << ".." << r.right() << ',' << r.top() << ']';
    return os;
}

// --------- binary save/load --------------------------------------------

static void writeU16LE(std::ostream& os, std::uint16_t v)
{
    unsigned char b[2] = { static_cast<unsigned char>(v & 0xFFu),
                           static_cast<unsigned char>((v >> 8) & 0xFFu) };
    os.write(reinterpret_cast<const char*>(b), 2);
}

static void writeI32LE(std::ostream& os, std::int32_t v)
{
    unsigned char b[4] = { static_cast<unsigned char>(v & 0xFF),
                           static_cast<unsigned char>((v >> 8) & 0xFF),
                           static_cast<unsigned char>((v >> 16) & 0xFF),
                           static_cast<unsigned char>((v >> 24) & 0xFF) };
    os.write(reinterpret_cast<const char*>(b), 4);
}

static bool readU16LE(std::istream& is, std::uint16_t& v)
{
    unsigned char b[2];
    if (!is.read(reinterpret_cast<char*>(b), 2)) {
        return false;
    }
    v = static_cast<std::uint16_t>(b[0] | (std::uint16_t(b[1]) << 8));
    return true;
}

static bool readI32LE(std::istream& is, std::int32_t& v)
{
    unsigned char b[4];
    if (!is.read(reinterpret_cast<char*>(b), 4)) {
        return false;
    }
    v = static_cast<std::int32_t>(b[0]
        | (std::int32_t(b[1]) << 8)
        | (std::int32_t(b[2]) << 16)
        | (std::int32_t(b[3]) << 24));
    return true;
}

bool saveBinary(std::ostream& os, const Mdn2d& mdn)
{
    // Header
    os.write(reinterpret_cast<const char*>(&kBinMagic), 4);
    writeU16LE(os, kBinVersion);
    writeU16LE(os, 0);

    int x0 = 0, y0 = 0, x1 = -1, y1 = -1;
    if (mdn.hasBounds()) {
        const Rect b = mdn.bounds();
        x0 = b.left(); y0 = b.bottom(); x1 = b.right(); y1 = b.top();
    }

    writeI32LE(os, x0);
    writeI32LE(os, y0);
    writeI32LE(os, x1);
    writeI32LE(os, y1);

    if (x1 < x0 || y1 < y0) {
        return static_cast<bool>(os); // empty
    }

    const int W = (x1 - x0 + 1);
    const int H = (y1 - y0 + 1);

    std::vector<Digit> row; row.reserve(static_cast<std::size_t>(W));
    for (int y = y0; y <= y1; ++y) {
        if (!mdn.getRow(y, x0, x1, row)) {
            row.assign(static_cast<std::size_t>(W), Digit(0));
        }
        // Write raw bytes of Digit (int8_t)
        os.write(reinterpret_cast<const char*>(row.data()), static_cast<std::streamsize>(row.size()));
    }

    return static_cast<bool>(os);
}

bool loadBinary(std::istream& is, Mdn2d& mdn)
{
    std::uint32_t magic = 0;
    if (!is.read(reinterpret_cast<char*>(&magic), 4)) {
        return false;
    }
    if (magic != kBinMagic) {
        return false;
    }

    std::uint16_t ver = 0, reserved = 0;
    if (!readU16LE(is, ver) || !readU16LE(is, reserved)) {
        return false;
    }
    if (ver != kBinVersion) {
        return false; // future-proofing: handle upgrades later
    }

    std::int32_t x0 = 0, y0 = 0, x1 = -1, y1 = -1;
    if (!readI32LE(is, x0) || !readI32LE(is, y0) || !readI32LE(is, x1) || !readI32LE(is, y1)) {
        return false;
    }

    mdn.clear();

    if (x1 < x0 || y1 < y0) {
        return true; // empty grid
    }

    const int W = (x1 - x0 + 1);
    const int H = (y1 - y0 + 1);

    std::vector<Digit> row; row.resize(static_cast<std::size_t>(W));

    for (int r = 0; r < H; ++r) {
        if (!is.read(reinterpret_cast<char*>(row.data()), static_cast<std::streamsize>(row.size()))) {
            return false;
        }
        mdn.setRowRange(y0 + r, x0, row);
    }

    return true;
}

} // namespace mdn::serialize
