#include <mdn/Mdn2d.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

// Exact text input and unlimited precision: the cascade index is the only
// truncation. An index N includes the initial digit and N additional terms.
static void emit(const char* name, mdn::Mdn2d& number, bool& separator) {
    if (separator) std::cout << ',';
    separator = true;
    std::cout << '"' << name << "\":[";
    bool cellSeparator = false;
    for (const auto& [coordinate, digit] : number.data_raw()) {
        if (cellSeparator) std::cout << ',';
        cellSeparator = true;
        std::cout << '[' << coordinate.x() << ',' << coordinate.y() << ','
                  << static_cast<int>(digit) << ']';
    }
    std::cout << ']';
}

int main(int argc, char** argv) {
    try {
        if (argc != 7) {
            throw std::invalid_argument("Expected base sign N M P Q");
        }
        mdn::Logger::instance().setEnabled(false);
        const int base = std::stoi(argv[1]);
        const auto sign = static_cast<mdn::SignConvention>(std::stoi(argv[2]));
        const int n = std::stoi(argv[3]), m = std::stoi(argv[4]);
        const int p = std::stoi(argv[5]), q = std::stoi(argv[6]);
        if (base < 2 || base > 16 || n < 0 || m < 0 || p < 0 || q < 0) {
            throw std::invalid_argument("Require 2 <= base <= 16 and nonnegative indices");
        }
        const mdn::Coord origin(0, 0);
        const mdn::Mdn2dConfig cn(base, -1, sign, n, mdn::Fraxis::X);
        const mdn::Mdn2dConfig cm(base, -1, sign, m, mdn::Fraxis::Y);
        const mdn::Mdn2dConfig cp(base, -1, sign, p, mdn::Fraxis::X);
        const mdn::Mdn2dConfig cq(base, -1, sign, q, mdn::Fraxis::Y);
        mdn::Mdn2d ln(cn), rm(cm), lp(cp), rq(cq);
        ln.add(origin, false, "0", "1", false, mdn::Fraxis::X);
        rm.add(origin, false, "0", "1", false, mdn::Fraxis::Y);
        lp.add(origin, false, "0", "1", false, mdn::Fraxis::X);
        rq.add(origin, false, "0", "1", false, mdn::Fraxis::Y);

        mdn::Mdn2d f(cn), fx(cn), fy(cn), f2(cn);
        ln.multiply(rm, f);
        f.multiply(lp, fx);
        f.multiply(rq, fy);
        f.multiply(f, f2);

        mdn::Mdn2d ll(cn), rr(cn), ln2(cn), rm2(cn);
        mdn::Mdn2d fxAlt(cn), fyAlt(cn), f2Alt(cn);
        ln.multiply(lp, ll);
        rm.multiply(rq, rr);
        ll.multiply(rm, fxAlt);
        ln.multiply(rr, fyAlt);
        ln.multiply(ln, ln2);
        rm.multiply(rm, rm2);
        ln2.multiply(rm2, f2Alt);

        bool separator = false;
        std::cout << '{';
        emit("L_N", ln, separator); emit("R_M", rm, separator);
        emit("L_P", lp, separator); emit("R_Q", rq, separator);
        emit("F", f, separator); emit("FX", fx, separator);
        emit("FY", fy, separator); emit("F2", f2, separator);
        emit("FX_alternate", fxAlt, separator);
        emit("FY_alternate", fyAlt, separator);
        emit("F2_alternate", f2Alt, separator);
        f.multiply(base * base);
        fx.multiply(base * base * base);
        fy.multiply(base * base * base);
        f2.multiply(base * base * base * base);
        emit("b2F", f, separator); emit("b3FX", fx, separator);
        emit("b3FY", fy, separator); emit("b4F2", f2, separator);
        std::cout << "}\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
