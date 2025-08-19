#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cassert>

// MDN headers
#include "Logger.h"
#include "Mdn2d.h"
#include "Mdn2dBase.h"
#include "Mdn2dConfig.h"
#include "Mdn2dIO.h"
#include "Rect.h"
#include "Digit.h"
#include "Tools.h"

// using mdn::Mdn2d;
// using mdn::Mdn2dBase;
// using mdn::Mdn2dIO;
// using mdn::TextWriteOptions;
// using mdn::CommaTabSpace;
// using mdn::AxesOutput;
// using mdn::Rect;
// using mdn::Digit;

using namespace mdn;

// ----- helpers -------------------------------------------------

static void printSection(const std::string& title) {
    Log_Info("" << "\n=== " << title << " ===");
}

static void printLines(const std::vector<std::string>& lines, bool topDown=false) {
    if (topDown) {
        for (auto it = lines.rbegin(); it != lines.rend(); ++it) {
            Log_Info("" << *it);
        }
    } else {
        for (const std::string& s : lines) {
            Log_Info("" << s);
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
    mdn::Mdn2dConfig cfg = m.config();
    mdn::Mdn2dConfig newCfg(10, 16, mdn::SignConvention::Positive, 20, mdn::Fraxis::X);
    m.setConfig(newCfg);

    // Define a 7x5 rectangle from (-2,-1) to (4,3)
    const Rect r(-2, -1, 4, 3, true);

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
    Log_Debug_H("roundTripTextPretty");

    std::stringstream ss;
    // operator<< emits pretty text via Mdn2dIO::saveTextPretty with defaults
    Log_Debug("stream out Mdn2dBase");
    Log_Info("roundTripTextPretty, operator<<");
    ss << src;

    Log_Debug("null ctor");
    Mdn2d dst;
    // operator>> dispatches Mdn2dIO::load (sniffs binary then falls back to text)
    Log_Info("ss text = [" << ss.str() << "]");
    ss.clear();
    ss.seekg(0, std::ios::beg);

    Log_Debug("stream in to Mdn2d");
    Log_Info("roundTripTextPretty, operator>>");
    ss >> dst;
    Log_Debug_T("roundTripTextPretty");
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
    mdn::Logger& sirTalksALot = mdn::Logger::instance();
    sirTalksALot.setLevel(mdn::LogLevel::Info);
    sirTalksALot.setOutputToFile();

    Log_Info("" << "MDN sandbox: text I/O smoke tests");

    // Build a sample MDN
    Log_Debug_H("");
    Mdn2d a;
    populateSample(a);
    Log_Debug_T("");

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
    Log_Debug_H("");
        printSection("Utility rows (space)");
        auto lines =
            Mdn2dIO::toStringRows(a, TextWriteOptions::DefaultUtility(CommaTabSpace::Space));
        printLines(lines, true);
    Log_Debug_T("");

    Log_Debug_H("");
        printSection("Utility rows (comma)");
        auto linesC = Mdn2dIO::toStringRows(a, TextWriteOptions::DefaultUtility(CommaTabSpace::Comma));
        printLines(linesC, true);
    Log_Debug_T("");

    Log_Debug_H("");
        printSection("Utility rows (tab)");
        auto linesT = Mdn2dIO::toStringRows(a, TextWriteOptions::DefaultUtility(CommaTabSpace::Tab));
        printLines(linesT, true);
    Log_Debug_T("");
    }

    // Columns view (utility-style, numeric, delimited)
    {
    Log_Debug_H("");
        printSection("Columns (utility, comma)");
        TextWriteOptions opt = TextWriteOptions::DefaultUtility(CommaTabSpace::Comma);
        auto cols = Mdn2dIO::toStringCols(a, opt);
        printLines(cols, true);
    Log_Debug_T("");
    }

    // Windowed pretty (sub-rect)
    {
    Log_Debug_H("");
        printSection("Pretty rows (windowed -1..2 x 0..2)");
        TextWriteOptions opt = TextWriteOptions::DefaultPretty();
        opt.window = Rect(-1, 0, 2, 2, true);
        auto lines = Mdn2dIO::toStringRows(a, opt);
        printLines(lines, true);
    Log_Debug_T("");
    }

    // Text pretty round-trip equivalence
    {
    Log_Debug_H("");
        printSection("Round-trip: pretty text");
        Mdn2d b = roundTripTextPretty(a);
        bool sameB = (a == b);
        Log_Info(
            "\n"
            << (sameB ? "PASS" : "FAIL") << " — pretty text round-trip equals by utility rows\n"
        );
        Log_Info("Printing a");
        {
            TextWriteOptions opt = TextWriteOptions::DefaultPretty();
            auto lines = Mdn2dIO::toStringRows(a, opt);
            printLines(lines, true);
        }
        Log_Info("Printing b");
        {
            TextWriteOptions opt = TextWriteOptions::DefaultPretty();
            auto lines = Mdn2dIO::toStringRows(b, opt);
            printLines(lines, true);
        }

        // Show the blob that was parsed (optional)
        Log_Info("" << "\n[pretty blob parsed]\n" << prettyBlob(a));
    Log_Debug_T("");
    }

    // Binary round-trip equivalence
    {
    Log_Debug_H("");
        printSection("Round-trip: binary");
        Mdn2d c = roundTripBinary(a);
        bool same = (a == c);
        Log_Info("" << (same ? "PASS" : "FAIL") << " — binary round-trip equals by utility rows");
        {
            TextWriteOptions opt = TextWriteOptions::DefaultPretty();
            auto lines = Mdn2dIO::toStringRows(a, opt);
            printLines(lines, true);
        }
        {
            TextWriteOptions opt = TextWriteOptions::DefaultPretty();
            auto lines = Mdn2dIO::toStringRows(c, opt);
            printLines(lines, true);
        }
    Log_Debug_T("");
    }

    // Direct stream operators (<< >>) also covered by roundTripTextPretty
    {
    Log_Debug_H("");
        printSection("Operator<< >> demo");
        std::stringstream ss;
        Log_Info("operator<<");
        ss << a;
        Log_Info("ss text = [" << ss.str() << "]");
        ss.clear();
        ss.seekg(0, std::ios::beg);

        Mdn2d d;
        Log_Info("operator>>");
        ss >> d;
        bool same = a == d;
        Log_Info("" << (same ? "PASS" : "FAIL") << " — operator round-trip equals");
        {
            TextWriteOptions opt = TextWriteOptions::DefaultPretty();
            auto lines = Mdn2dIO::toStringRows(a, opt);
            printLines(lines, true);
        }
        {
            TextWriteOptions opt = TextWriteOptions::DefaultPretty();
            auto lines = Mdn2dIO::toStringRows(d, opt);
            printLines(lines, true);
        }
    Log_Debug_T("");
    }

    // Explicit loader from a synthetic “pretty” string with axes and “Bounds = …” footer
    {
    Log_Debug_H("");
        printSection("LoadText from pretty-ish blob (axes + Bounds line)");
        std::string blob = prettyBlob(a) + "\nBounds = [(-2,-1) -> (4,3)]\n";
        std::stringstream ss(blob);
        Mdn2d e;
        Mdn2dIO::loadText(ss, e);
        bool same = a == e;
        Log_Info("" << (same ? "PASS" : "FAIL") << " — loadText handled axes + footer");
        {
            TextWriteOptions opt = TextWriteOptions::DefaultPretty();
            auto lines = Mdn2dIO::toStringRows(a, opt);
            printLines(lines, true);
        }
        {
            TextWriteOptions opt = TextWriteOptions::DefaultPretty();
            auto lines = Mdn2dIO::toStringRows(e, opt);
            printLines(lines, true);
        }
    Log_Debug_T("");
    }

    Log_Info("" << "\nDone.");
    return 0;
}
