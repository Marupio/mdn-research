#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cassert>

// MDN headers
#include "Mdn2d.h"
#include "Mdn2dBase.h"
#include "Mdn2dConfig.h"
#include "Mdn2dIO.h"
#include "Rect.h"
#include "Digit.h"
#include "Tools.h"

using mdn::Mdn2d;
using mdn::Mdn2dBase;
using mdn::Mdn2dIO;
using mdn::TextWriteOptions;
using mdn::CommaTabSpace;
using mdn::AxesOutput;
using mdn::Rect;
using mdn::Digit;

// ----- helpers -------------------------------------------------

static void printSection(const std::string& title) {
    std::cout << "\n=== " << title << " ===\n";
}

static void printLines(const std::vector<std::string>& lines, bool topDown=false) {
    if (topDown) {
        for (auto it = lines.rbegin(); it != lines.rend(); ++it) {
            std::cout << *it << '\n';
        }
    } else {
        for (const std::string& s : lines) {
            std::cout << s << '\n';
        }
    }
}


static std::vector<Digit> makeRow(std::initializer_list<int> vals) {
    std::vector<Digit> row;
    row.reserve(vals.size());
    for (int v : vals) {
        row.push_back(static_cast<Digit>(v));
    }
    return row;
}

// Compare two MDNs by utility text (stable, delimiter-agnostic via Space)
static bool equalByUtilityText(const Mdn2dBase& a, const Mdn2dBase& b) {
    TextWriteOptions oa = TextWriteOptions::DefaultUtility(CommaTabSpace::Space);
    TextWriteOptions ob = TextWriteOptions::DefaultUtility(CommaTabSpace::Space);
    return Mdn2dIO::toStringRows(a, oa) == Mdn2dIO::toStringRows(b, ob);
}

// Sample content: a small 7x5 window with negatives and alpha-range digits
static void populateSample(Mdn2dBase& m) {
    // Configure base and a couple of defaults
    mdn::Mdn2dConfig cfg = m.getConfig();
    mdn::Mdn2dConfig newCfg(16, 20, mdn::SignConvention::Positive, 20, mdn::Fraxis::X);
    m.setConfig(newCfg);

    // Define a 7x5 rectangle from (-2,-1) to (4,3)
    const Rect r(-2, -1, 4, 3, /*fixOrdering*/true);

    // y = -1
    {
        auto row = makeRow({-1, 0, 1, 0, 0, 0, 0});
        m.setRow(-1, -2, row);
    }
    // y = 0
    {
        auto row = makeRow({0, 0, 0, 3, 0, 0, -8});
        m.setRow( 0, -2, row);
    }
    // y = 1
    {
        auto row = makeRow({1, 0, 0, 0, 3, 0, -8});
        m.setRow( 1, -2, row);
    }
    // y = 2
    {
        auto row = makeRow({2, 0, 0, 0, 0, 1, -8});
        m.setRow( 2, -2, row);
    }
    // y = 3  (put some negatives and alpha-range)
    {
        // {..., -3, 0, 10(a), 0, 0}
        auto row = makeRow({3, 4, 5, 6, 7, 8, 9});
        m.setRow( 3, -2, row);
    }

    // optional: set a name if your class exposes it (nice for copy “origin”)
    // m.setName("Sample");
    (void)r; // r illustrates intended bounds; Mdn2dBase derives bounds from set data
}

// Round-trip binary and return a new instance
static Mdn2d roundTripBinary(const Mdn2dBase& src) {
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    Mdn2dIO::saveBinary(src, ss);

    Mdn2d dst;
    Mdn2dIO::loadBinary(ss, dst);
    return dst;
}

// Round-trip pretty text (ASCII) and return a new instance
static Mdn2d roundTripTextPretty(const Mdn2dBase& src) {
    std::stringstream ss;
    // operator<< emits pretty text via Mdn2dIO::saveTextPretty with defaults
    ss << src;

    Mdn2d dst;
    // operator>> dispatches Mdn2dIO::load (sniffs binary then falls back to text)
    ss >> dst;
    return dst;
}

// Build a pretty text blob (with axes) from current MDN
static std::string prettyBlob(const Mdn2dBase& m) {
    TextWriteOptions opt = TextWriteOptions::DefaultPretty();
    std::vector<std::string> lines = Mdn2dIO::toStringRows(m, opt);
    std::ostringstream os;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        os << lines[i];
        if (i + 1 < lines.size()) {
            os << '\n';
        }
    }
    return os.str();
}

// ----- main tests ---------------------------------------------

int main() {
    std::cout << "MDN sandbox: text I/O smoke tests\n";

    // Build a sample MDN
    Mdn2d a;
    populateSample(a);

    // Mdn2d slot0 = Mdn2d::NewInstance(Mdn2dConfig(10, 32));
    // slot0.setValue(COORD_ORIGIN, 3);
    // slot0.setValue(Coord(0, 1), 2);
    // slot0.setValue(Coord(1, 0), -2);
    // slot0.setValue(Coord(1, 1), 1);
    // slot0.setValue(Coord(20, 6), 9);
    //
    // std::vector<std::string> slot0Disp = slot0.toStringRows();
    // for (auto riter = slot0Disp.rbegin(); riter != slot0Disp.rend(); ++riter) {
    //     std::cout << *riter << '\n';
    // }
    // std::cout << "Bounds = " << slot0.bounds() << std::endl;

    // Pretty rows (box-art axes, alphanumerics, wide negatives)
    {
        printSection("Pretty rows (DefaultPretty)");
        TextWriteOptions opt = TextWriteOptions::DefaultPretty();
        auto lines = Mdn2dIO::toStringRows(a, opt);
        printLines(lines, true);

        std::cout << '\n';

        std::vector<std::string> rows(a.toStringRows());
        std::vector<std::string>::const_iterator iter;
        for (auto riter = rows.rbegin(); riter != rows.rend(); ++riter) {
            std::cout << *riter << '\n';
        }
        std::cout << "Bounds = " << a.bounds() << std::endl;
    }

    // Utility rows with various delimiters
    {
        printSection("Utility rows (space)");
        auto lines = Mdn2dIO::toStringRows(a, TextWriteOptions::DefaultUtility(CommaTabSpace::Space));
        printLines(lines, true);

        printSection("Utility rows (comma)");
        auto linesC = Mdn2dIO::toStringRows(a, TextWriteOptions::DefaultUtility(CommaTabSpace::Comma));
        printLines(linesC, true);

        printSection("Utility rows (tab)");
        auto linesT = Mdn2dIO::toStringRows(a, TextWriteOptions::DefaultUtility(CommaTabSpace::Tab));
        printLines(linesT, true);
    }

    // Columns view (utility-style, numeric, delimited)
    {
        printSection("Columns (utility, comma)");
        TextWriteOptions opt = TextWriteOptions::DefaultUtility(CommaTabSpace::Comma);
        auto cols = Mdn2dIO::toStringCols(a, opt);
        printLines(cols, true);
    }

    // Windowed pretty (sub-rect)
    {
        printSection("Pretty rows (windowed -1..2 x 0..2)");
        TextWriteOptions opt = TextWriteOptions::DefaultPretty();
        opt.window = Rect(-1, 0, 2, 2, /*fixOrdering*/true);
        auto lines = Mdn2dIO::toStringRows(a, opt);
        printLines(lines, true);
    }

    // Text pretty round-trip equivalence
    {
        printSection("Round-trip: pretty text");
        Mdn2d b = roundTripTextPretty(a);
        bool same = equalByUtilityText(a, b);
        std::cout << (same ? "PASS" : "FAIL") << " — pretty text round-trip equals by utility rows\n";

        // Show the blob that was parsed (optional)
        std::cout << "\n[pretty blob parsed]\n" << prettyBlob(a) << "\n";
    }

    // Binary round-trip equivalence
    {
        printSection("Round-trip: binary");
        Mdn2d c = roundTripBinary(a);
        bool same = equalByUtilityText(a, c);
        std::cout << (same ? "PASS" : "FAIL") << " — binary round-trip equals by utility rows\n";
    }

    // Direct stream operators (<< >>) also covered by roundTripTextPretty
    {
        printSection("Operator<< >> demo");
        std::stringstream ss;
        ss << a;
        Mdn2d d;
        ss >> d;
        bool same = equalByUtilityText(a, d);
        std::cout << (same ? "PASS" : "FAIL") << " — operator round-trip equals\n";
    }

    // Explicit loader from a synthetic “pretty” string with axes and “Bounds = …” footer
    {
        printSection("LoadText from pretty-ish blob (axes + Bounds line)");
        std::string blob = prettyBlob(a) + "\nBounds = [(-2,-1) -> (4,3)]\n";
        std::stringstream ss(blob);
        Mdn2d e;
        Mdn2dIO::loadText(ss, e);
        bool same = equalByUtilityText(a, e);
        std::cout << (same ? "PASS" : "FAIL") << " — loadText handled axes + footer\n";
    }

    std::cout << "\nDone.\n";
    return 0;
}
